// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_builder.h"

#include <algorithm>

#include "base/notimplemented.h"
#include "base/numerics/checked_math.h"
#include "base/ranges/algorithm.h"
#include "base/types/expected.h"
#include "base/types/expected_macros.h"
#include "components/ml/webnn/features.mojom-blink.h"
#include "services/webnn/public/mojom/webnn_graph.mojom-blink.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_arg_min_max_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_batch_normalization_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_clamp_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_conv_2d_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_conv_transpose_2d_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_elu_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_gather_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_gemm_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_gru_cell_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_gru_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_hard_sigmoid_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_instance_normalization_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_layer_normalization_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_leaky_relu_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_linear_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_lstm_cell_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_lstm_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_operand_descriptor.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_pad_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_pool_2d_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_reduce_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_resample_2d_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_split_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_transpose_options.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_triangular_options.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/ml/buildflags.h"
#include "third_party/blink/renderer/modules/ml/ml_context.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_activation.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_mojo.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_utils.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_operand.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_operator.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"

#if BUILDFLAG(BUILD_WEBNN_WITH_XNNPACK)
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_xnnpack.h"
#endif

namespace blink {

namespace {

#define THROW_AND_RETURN_TYPE_IF_ERROR(func, return_value) \
  RETURN_IF_ERROR(func, [&exception_state](String error) { \
    exception_state.ThrowTypeError(error);                 \
    return return_value;                                   \
  });

MLGraphBuilder::BackendForTesting* g_backend_for_testing = nullptr;

blink::V8MLOperandDataType::Enum ComponentOperandTypeToBlink(
    webnn::Operand::DataType data_type) {
  switch (data_type) {
    case webnn::Operand::DataType::kFloat32:
      return blink::V8MLOperandDataType::Enum::kFloat32;
    case webnn::Operand::DataType::kFloat16:
      return blink::V8MLOperandDataType::Enum::kFloat16;
    case webnn::Operand::DataType::kInt32:
      return blink::V8MLOperandDataType::Enum::kInt32;
    case webnn::Operand::DataType::kUint32:
      return blink::V8MLOperandDataType::Enum::kUint32;
    case webnn::Operand::DataType::kInt64:
      return blink::V8MLOperandDataType::Enum::kInt64;
    case webnn::Operand::DataType::kUint64:
      return blink::V8MLOperandDataType::Enum::kUint64;
    case webnn::Operand::DataType::kInt8:
      return blink::V8MLOperandDataType::Enum::kInt8;
    case webnn::Operand::DataType::kUint8:
      return blink::V8MLOperandDataType::Enum::kUint8;
  }
  NOTREACHED_NORETURN();
}

webnn::Operand::DataType BlinkOperandTypeToComponent(
    blink::V8MLOperandDataType::Enum data_type) {
  switch (data_type) {
    case blink::V8MLOperandDataType::Enum::kFloat32:
      return webnn::Operand::DataType::kFloat32;
    case blink::V8MLOperandDataType::Enum::kFloat16:
      return webnn::Operand::DataType::kFloat16;
    case blink::V8MLOperandDataType::Enum::kInt32:
      return webnn::Operand::DataType::kInt32;
    case blink::V8MLOperandDataType::Enum::kUint32:
      return webnn::Operand::DataType::kUint32;
    case blink::V8MLOperandDataType::Enum::kInt64:
      return webnn::Operand::DataType::kInt64;
    case blink::V8MLOperandDataType::Enum::kUint64:
      return webnn::Operand::DataType::kUint64;
    case blink::V8MLOperandDataType::Enum::kInt8:
      return webnn::Operand::DataType::kInt8;
    case blink::V8MLOperandDataType::Enum::kUint8:
      return webnn::Operand::DataType::kUint8;
  }
  NOTREACHED_NORETURN();
}

webnn::Operand ConvertToComponentOperand(const blink::MLOperand* ml_operand) {
  return webnn::Operand(BlinkOperandTypeToComponent(ml_operand->DataType()),
                        ml_operand->Dimensions());
}

webnn::InputOperandLayout BlinkInputOperandLayoutToComponent(
    blink::V8MLInputOperandLayout::Enum type) {
  switch (type) {
    case blink::V8MLInputOperandLayout::Enum::kNchw:
      return webnn::InputOperandLayout::kNchw;
    case blink::V8MLInputOperandLayout::Enum::kNhwc:
      return webnn::InputOperandLayout::kNhwc;
  }
  NOTREACHED_NORETURN();
}

webnn::Conv2dFilterOperandLayout BlinkConv2dFilterLayoutToComponent(
    blink::V8MLConv2dFilterOperandLayout::Enum type) {
  switch (type) {
    case blink::V8MLConv2dFilterOperandLayout::Enum::kOihw:
      return webnn::Conv2dFilterOperandLayout::kOihw;
    case blink::V8MLConv2dFilterOperandLayout::Enum::kHwio:
      return webnn::Conv2dFilterOperandLayout::kHwio;
    case blink::V8MLConv2dFilterOperandLayout::Enum::kOhwi:
      return webnn::Conv2dFilterOperandLayout::kOhwi;
    case blink::V8MLConv2dFilterOperandLayout::Enum::kIhwo:
      return webnn::Conv2dFilterOperandLayout::kIhwo;
  }
  NOTREACHED_NORETURN();
}

webnn::ConvTranspose2dFilterOperandLayout
BlinkConvTranspose2dFilterLayoutToComponent(
    blink::V8MLConvTranspose2dFilterOperandLayout::Enum type) {
  switch (type) {
    case blink::V8MLConvTranspose2dFilterOperandLayout::Enum::kIohw:
      return webnn::ConvTranspose2dFilterOperandLayout::kIohw;
    case blink::V8MLConvTranspose2dFilterOperandLayout::Enum::kHwoi:
      return webnn::ConvTranspose2dFilterOperandLayout::kHwoi;
    case blink::V8MLConvTranspose2dFilterOperandLayout::Enum::kOhwi:
      return webnn::ConvTranspose2dFilterOperandLayout::kOhwi;
  }
  NOTREACHED_NORETURN();
}

webnn::RoundingType BlinkRoundingTypeToComponent(
    blink::V8MLRoundingType::Enum type) {
  switch (type) {
    case blink::V8MLRoundingType::Enum::kFloor:
      return webnn::RoundingType::kFloor;
    case blink::V8MLRoundingType::Enum::kCeil:
      return webnn::RoundingType::kCeil;
  }
  NOTREACHED_NORETURN();
}

webnn::ReduceKind MojoReduceKindToComponent(
    webnn::mojom::blink::Reduce::Kind kind) {
  switch (kind) {
    case webnn::mojom::blink::Reduce::Kind::kL1:
      return webnn::ReduceKind::kL1;
    case webnn::mojom::blink::Reduce::Kind::kL2:
      return webnn::ReduceKind::kL2;
    case webnn::mojom::blink::Reduce::Kind::kLogSum:
      return webnn::ReduceKind::kLogSum;
    case webnn::mojom::blink::Reduce::Kind::kLogSumExp:
      return webnn::ReduceKind::kLogSumExp;
    case webnn::mojom::blink::Reduce::Kind::kMax:
      return webnn::ReduceKind::kMax;
    case webnn::mojom::blink::Reduce::Kind::kMean:
      return webnn::ReduceKind::kMean;
    case webnn::mojom::blink::Reduce::Kind::kMin:
      return webnn::ReduceKind::kMin;
    case webnn::mojom::blink::Reduce::Kind::kProduct:
      return webnn::ReduceKind::kProduct;
    case webnn::mojom::blink::Reduce::Kind::kSum:
      return webnn::ReduceKind::kSum;
    case webnn::mojom::blink::Reduce::Kind::kSumSquare:
      return webnn::ReduceKind::kSumSquare;
  }
}

webnn::RecurrentNetworkDirection BlinkRecurrentNetworkDirectionToComponent(
    blink::V8MLRecurrentNetworkDirection::Enum direction) {
  switch (direction) {
    case blink::V8MLRecurrentNetworkDirection::Enum::kForward:
      return webnn::RecurrentNetworkDirection::kForward;
    case blink::V8MLRecurrentNetworkDirection::Enum::kBackward:
      return webnn::RecurrentNetworkDirection::kBackward;
    case blink::V8MLRecurrentNetworkDirection::Enum::kBoth:
      return webnn::RecurrentNetworkDirection::kBoth;
  }
  NOTREACHED_NORETURN();
}

webnn::BatchNormalizationAttributes ConvertToBatchNormalizationAttributes(
    const blink::MLBatchNormalizationOptions* options) {
  CHECK(options);
  webnn::BatchNormalizationAttributes attributes;
  if (options->hasScale()) {
    attributes.scale = ConvertToComponentOperand(options->scale());
  }
  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  attributes.axis = options->axis();
  return attributes;
}

template <typename MLConv2dOptionsType, typename Conv2dAttributesType>
base::expected<Conv2dAttributesType, String> ConvertToConv2dAttributesBase(
    const MLConv2dOptionsType* options) {
  Conv2dAttributesType attributes;
  CHECK(options);
  // If padding is not present, the values are assumed to be [0,0,0,0].
  auto padding = options->getPaddingOr({0, 0, 0, 0});
  if (padding.size() != 4) {
    return base::unexpected("The length of padding should be 4.");
  }
  // The order of padding array is [beginning_height, ending_height,
  // beginning_width, ending_width].
  attributes.padding = webnn::Padding2d{
      .beginning =
          webnn::Size2d<uint32_t>{.height = padding[0], .width = padding[2]},
      .ending =
          webnn::Size2d<uint32_t>{.height = padding[1], .width = padding[3]}};

  // If strides is not present, the values are assumed to be [1,1].
  auto strides = options->getStridesOr({1, 1});
  if (strides.size() != 2) {
    return base::unexpected("The length of strides should be 2.");
  }
  attributes.strides =
      webnn::Size2d<uint32_t>{.height = strides[0], .width = strides[1]};

  // If dilations is not present, the values are assumed to be [1,1].
  auto dilations = options->getDilationsOr({1, 1});
  if (dilations.size() != 2) {
    return base::unexpected("The length of dilations should be 2.");
  }
  attributes.dilations =
      webnn::Size2d<uint32_t>{.height = dilations[0], .width = dilations[1]};
  attributes.groups = options->groups();
  attributes.input_layout =
      BlinkInputOperandLayoutToComponent(options->inputLayout().AsEnum());
  if (options->hasBias()) {
    attributes.bias_operand = ConvertToComponentOperand(options->bias());
  }

  return std::move(attributes);
}

base::expected<webnn::Conv2dAttributes, String> ConvertToConv2dAttributes(
    const blink::MLConv2dOptions* options) {
  auto attributes =
      ConvertToConv2dAttributesBase<blink::MLConv2dOptions,
                                    webnn::Conv2dAttributes>(options);
  if (!attributes.has_value()) {
    return base::unexpected(attributes.error());
  }
  attributes.value().filter_layout =
      BlinkConv2dFilterLayoutToComponent(options->filterLayout().AsEnum());

  return attributes;
}

base::expected<webnn::ConvTranspose2dAttributes, String>
ConvertToConvTranspose2dAttributes(
    const blink::MLConvTranspose2dOptions* options) {
  auto attributes =
      ConvertToConv2dAttributesBase<blink::MLConvTranspose2dOptions,
                                    webnn::ConvTranspose2dAttributes>(options);
  if (!attributes.has_value()) {
    return base::unexpected(attributes.error());
  }

  // If output padding is not present, the values are assumed to be [0,0].
  const auto output_padding = options->getOutputPaddingOr({0, 0});
  if (output_padding.size() != 2) {
    return base::unexpected("The length of output padding should be 2.");
  }
  attributes.value().output_padding = webnn::Size2d<uint32_t>{
      .height = output_padding[0], .width = output_padding[1]};

  if (options->hasOutputSizes()) {
    auto output_sizes = options->getOutputSizesOr({});
    if (output_sizes.size() != 2) {
      return base::unexpected("The length of output sizes should be 2.");
    }
    attributes.value().output_sizes = webnn::Size2d<uint32_t>{
        .height = output_sizes[0], .width = output_sizes[1]};
  }

  attributes.value().filter_layout =
      BlinkConvTranspose2dFilterLayoutToComponent(
          options->filterLayout().AsEnum());

  return attributes;
}

base::expected<webnn::Pool2dAttributes, std::string> ConvertToPool2dAttributes(
    const blink::MLPool2dOptions* options) {
  CHECK(options);
  webnn::Pool2dAttributes attributes;
  if (options->hasWindowDimensions()) {
    auto& window_dimensions = options->windowDimensions();
    if (window_dimensions.size() != 2) {
      return base::unexpected("The length of window dimensions should be 2.");
    }
    attributes.window_dimensions = webnn::Size2d<uint32_t>{
        .height = window_dimensions[0], .width = window_dimensions[1]};
  }

  // If padding is not present, the values are assumed to be [0,0,0,0].
  auto padding = options->getPaddingOr({0, 0, 0, 0});
  if (padding.size() != 4) {
    return base::unexpected("The length of padding should be 4.");
  }
  attributes.padding = webnn::Padding2d{
      .beginning =
          webnn::Size2d<uint32_t>{.height = padding[0], .width = padding[2]},
      .ending =
          webnn::Size2d<uint32_t>{.height = padding[1], .width = padding[3]}};

  // If strides is not present, the values are assumed to be [1,1].
  auto strides = options->getStridesOr({1, 1});
  if (strides.size() != 2) {
    return base::unexpected("The length of strides should be 2.");
  }
  attributes.strides =
      webnn::Size2d<uint32_t>{.height = strides[0], .width = strides[1]};

  // If dilations is not present, the values are assumed to be [1,1].
  auto dilations = options->getDilationsOr({1, 1});
  if (dilations.size() != 2) {
    return base::unexpected("The length of dilations should be 2.");
  }
  attributes.dilations =
      webnn::Size2d<uint32_t>{.height = dilations[0], .width = dilations[1]};
  attributes.layout =
      BlinkInputOperandLayoutToComponent(options->layout().AsEnum());
  attributes.rounding_type =
      BlinkRoundingTypeToComponent(options->roundingType().AsEnum());
  if (options->hasOutputSizes()) {
    // TODO(ningxin.hu@intel.com): report a DevTools warning message if rounding
    // type is provided but ignored.
    auto& output_size = options->outputSizes();
    if (output_size.size() != 2) {
      return base::unexpected("The length of output sizes should be 2.");
    }
    attributes.output_sizes = webnn::Size2d<uint32_t>{.height = output_size[0],
                                                      .width = output_size[1]};
  }
  return attributes;
}

webnn::GemmAttributes ConvertToGemmAttributes(
    const blink::MLGemmOptions* options) {
  CHECK(options);
  webnn::GemmAttributes attributes;
  if (options->hasC()) {
    attributes.c_operand = ConvertToComponentOperand(options->c());
  }
  attributes.alpha = options->alpha();
  attributes.beta = options->beta();
  attributes.a_transpose = options->aTranspose();
  attributes.b_transpose = options->bTranspose();
  return attributes;
}

webnn::GruAttributes ConvertToGruAttributes(MLGraphBuilder* builder,
                                            blink::MLGruOptions* options) {
  CHECK(options);
  webnn::GruAttributes attributes;

  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  if (options->hasRecurrentBias()) {
    attributes.recurrent_bias =
        ConvertToComponentOperand(options->recurrentBias());
  }
  if (options->hasInitialHiddenState()) {
    attributes.initial_hidden_state =
        ConvertToComponentOperand(options->initialHiddenState());
  }
  attributes.return_sequence = options->returnSequence();
  attributes.direction =
      BlinkRecurrentNetworkDirectionToComponent(options->direction().AsEnum());
  // If the activations are not specified, create a default activation sequence
  // [sigmoid, tanh] as defined in the spec.
  if (!options->hasActivations()) {
    MLActivation* activation_sigmoid = MakeGarbageCollected<MLActivation>(
        builder, webnn::mojom::blink::Activation::Tag::kSigmoid);
    MLActivation* activation_tanh = MakeGarbageCollected<MLActivation>(
        builder, webnn::mojom::blink::Activation::Tag::kTanh);
    options->setActivations({activation_sigmoid, activation_tanh});
  }
  attributes.activation_count = options->activations().size();

  return attributes;
}

webnn::GruCellAttributes ConvertToGruCellAttributes(
    MLGraphBuilder* builder,
    blink::MLGruCellOptions* options) {
  CHECK(options);
  webnn::GruCellAttributes attributes;

  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  if (options->hasRecurrentBias()) {
    attributes.recurrent_bias =
        ConvertToComponentOperand(options->recurrentBias());
  }
  // If the activations are not specified, create a default activation sequence
  // [sigmoid, tanh] as defined in the spec.
  if (!options->hasActivations()) {
    MLActivation* activation_sigmoid = MakeGarbageCollected<MLActivation>(
        builder, webnn::mojom::blink::Activation::Tag::kSigmoid);
    MLActivation* activation_tanh = MakeGarbageCollected<MLActivation>(
        builder, webnn::mojom::blink::Activation::Tag::kTanh);
    options->setActivations({activation_sigmoid, activation_tanh});
  }
  attributes.activation_count = options->activations().size();

  return attributes;
}

webnn::InstanceNormalizationAttributes ConvertToInstanceNormalizationAttributes(
    const blink::MLInstanceNormalizationOptions* options) {
  CHECK(options);
  webnn::InstanceNormalizationAttributes attributes;
  if (options->hasScale()) {
    attributes.scale = ConvertToComponentOperand(options->scale());
  }
  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  attributes.layout =
      BlinkInputOperandLayoutToComponent(options->layout().AsEnum());
  return attributes;
}

webnn::LayerNormalizationAttributes ConvertToLayerNormalizationAttributes(
    const blink::MLLayerNormalizationOptions* options) {
  CHECK(options);
  webnn::LayerNormalizationAttributes attributes;
  if (options->hasScale()) {
    attributes.scale = ConvertToComponentOperand(options->scale());
  }
  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  return attributes;
}

webnn::LstmAttributes ConvertToLstmAttributes(
    const blink::MLLstmOptions* options) {
  CHECK(options);
  webnn::LstmAttributes attributes;

  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  if (options->hasRecurrentBias()) {
    attributes.recurrent_bias =
        ConvertToComponentOperand(options->recurrentBias());
  }
  if (options->hasPeepholeWeight()) {
    attributes.peephole_weight =
        ConvertToComponentOperand(options->peepholeWeight());
  }
  if (options->hasInitialHiddenState()) {
    attributes.initial_hidden_state =
        ConvertToComponentOperand(options->initialHiddenState());
  }
  if (options->hasInitialCellState()) {
    attributes.initial_cell_state =
        ConvertToComponentOperand(options->initialCellState());
  }
  attributes.activation_count = options->activations().size();
  attributes.return_sequence = options->returnSequence();
  attributes.direction =
      BlinkRecurrentNetworkDirectionToComponent(options->direction().AsEnum());

  return attributes;
}

webnn::LstmCellAttributes ConvertToLstmCellAttributes(
    const blink::MLLstmCellOptions* options) {
  CHECK(options);
  webnn::LstmCellAttributes attributes;

  if (options->hasBias()) {
    attributes.bias = ConvertToComponentOperand(options->bias());
  }
  if (options->hasRecurrentBias()) {
    attributes.recurrent_bias =
        ConvertToComponentOperand(options->recurrentBias());
  }
  if (options->hasPeepholeWeight()) {
    attributes.peephole_weight =
        ConvertToComponentOperand(options->peepholeWeight());
  }
  attributes.activation_count = options->activations().size();

  return attributes;
}

bool ValidateClampOptions(const MLClampOptions* options,
                          ExceptionState& exception_state) {
  // The generated code of MLClampOptions uses blink::ToRestrictedFloat to
  // convert the min/max value to a single precision float. It will throw on
  // non-finite values.
  if (options->hasMinValue() && options->hasMaxValue()) {
    if (options->minValue() > options->maxValue()) {
      exception_state.ThrowTypeError(
          String::Format("The min value (%f) should be less than or equal to "
                         "the max value (%f).",
                         options->minValue(), options->maxValue()));
      return false;
    }
  }
  return true;
}

std::optional<Vector<uint32_t>> BroadcastShapes(
    const Vector<uint32_t>& dims_lhs,
    const Vector<uint32_t>& dims_rhs,
    bool bidirectional = true) {
  auto output_shape = webnn::BroadcastShapes(
      base::make_span(dims_lhs), base::make_span(dims_rhs), bidirectional);
  if (!output_shape) {
    return std::nullopt;
  }
  return Vector<uint32_t>(output_shape.value());
}

constexpr bool IsLogicalBinaryOperator(
    webnn::mojom::blink::ElementWiseBinary::Kind kind) {
  switch (kind) {
    case webnn::mojom::blink::ElementWiseBinary::Kind::kAdd:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kSub:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kMul:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kDiv:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kMax:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kMin:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kPow:
      return false;
    case webnn::mojom::blink::ElementWiseBinary::Kind::kEqual:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kGreater:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kGreaterOrEqual:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kLesser:
    case webnn::mojom::blink::ElementWiseBinary::Kind::kLesserOrEqual:
      return true;
  }
}

MLOperand* BuildArgMinMax(MLGraphBuilder* builder,
                          webnn::mojom::blink::ArgMinMax::Kind kind,
                          const MLOperand* input,
                          const MLArgMinMaxOptions* options,
                          ExceptionState& exception_state) {
  const auto input_rank = input->Dimensions().size();
  const auto axes = options->getAxesOr(CreateAllAxes(input_rank));
  const auto validated_output = webnn::ValidateArgMinMaxAndInferOutput(
      ConvertToComponentOperand(input), axes, options->keepDimensions());
  if (!validated_output.has_value()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kDataError,
        WTF::String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* arg_min_max = MakeGarbageCollected<MLOperator>(
      builder, /*kind=*/webnn::mojom::blink::Operation::Tag::kArgMinMax,
      /*sub_kind=*/kind, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      builder, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), arg_min_max);
  if (!output.has_value()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kDataError,
                                      output.error());
    return nullptr;
  }
  arg_min_max->Connect({input}, {output.value()});

  return output.value();
}

MLOperand* BuildElementWiseBinary(
    MLGraphBuilder* builder,
    webnn::mojom::blink::ElementWiseBinary::Kind kind,
    const MLOperand* a,
    const MLOperand* b,
    ExceptionState& exception_state) {
  if (a->DataType() != b->DataType()) {
    exception_state.ThrowTypeError("The input operand data types don't match.");
    return nullptr;
  }
  std::optional<Vector<uint32_t>> dims_output =
      BroadcastShapes(a->Dimensions(), b->Dimensions());
  if (!dims_output) {
    exception_state.ThrowTypeError("The input shapes are not broadcastable.");
    return nullptr;
  }

  // Logical operator outputs are bools, otherwise output operators are the same
  // type as input operators.
  V8MLOperandDataType::Enum data_type = IsLogicalBinaryOperator(kind)
                                            ? V8MLOperandDataType::Enum::kUint8
                                            : a->DataType();

  auto* binary = MakeGarbageCollected<MLOperator>(
      builder, /*kind=*/webnn::mojom::blink::Operation::Tag::kElementWiseBinary,
      /*sub_kind=*/kind);
  auto output = MLOperand::ValidateAndCreateOutput(builder, data_type,
                                                   dims_output.value(), binary);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  binary->Connect({a, b}, {output.value()});
  return output.value();
}

MLOperand* BuildUnaryOperator(
    MLGraphBuilder* builder,
    ExceptionState& exception_state,
    webnn::mojom::blink::Operation::Tag kind,
    const webnn::DataTypeConstraintSet& data_type_constraint,
    const MLOperand* input,
    const bindings::DictionaryBase* options = nullptr) {
  // The output tensor of unary operator has the same data type and dimensions
  // as its input tensor.
  if (!data_type_constraint.Has(
          BlinkOperandTypeToComponent(input->DataType()))) {
    exception_state.ThrowTypeError(String::Format(
        "The input data type must be one of the %s types.",
        webnn::DataTypeConstraintToString(data_type_constraint).c_str()));
    return nullptr;
  }

  auto* unary =
      MakeGarbageCollected<MLOperator>(builder, kind,
                                       /*sub_kind=*/absl::monostate{}, options);
  auto output = MLOperand::ValidateAndCreateOutput(builder, input->DataType(),
                                                   input->Dimensions(), unary);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  unary->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* BuildElementWiseUnaryOperator(
    MLGraphBuilder* builder,
    ExceptionState& exception_state,
    webnn::mojom::blink::ElementWiseUnary::Kind kind,
    const webnn::DataTypeConstraintSet& data_type_constraint,
    const MLOperand* input) {
  // The output tensor of unary operator has the same data type and dimensions
  // as its input tensor.
  if (!data_type_constraint.Has(
          BlinkOperandTypeToComponent(input->DataType()))) {
    exception_state.ThrowTypeError(String::Format(
        "The input data type must be one of the %s types.",
        webnn::DataTypeConstraintToString(data_type_constraint).c_str()));
    return nullptr;
  }

  auto* unary = MakeGarbageCollected<MLOperator>(
      builder, /*kind=*/webnn::mojom::blink::Operation::Tag::kElementWiseUnary,
      /*sub_kind=*/kind);
  auto output = MLOperand::ValidateAndCreateOutput(builder, input->DataType(),
                                                   input->Dimensions(), unary);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  unary->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* BuildReduce(MLGraphBuilder* builder,
                       webnn::mojom::blink::Reduce::Kind kind,
                       const MLOperand* input,
                       const MLReduceOptions* options,
                       ExceptionState& exception_state) {
  const auto input_rank = input->Dimensions().size();
  const auto axes = options->getAxesOr(CreateAllAxes(input_rank));
  auto validated_output = webnn::ValidateReduceAndInferOutput(
      MojoReduceKindToComponent(kind), ConvertToComponentOperand(input), axes,
      options->keepDimensions());
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* reduce = MakeGarbageCollected<MLOperator>(
      builder, /*kind=*/webnn::mojom::blink::Operation::Tag::kReduce,
      /*sub_kind=*/kind, options);
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-reduce, the output
  // tensor of reduce has the same data type as its input.
  auto output = MLOperand::ValidateAndCreateOutput(
      builder, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), reduce);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  reduce->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* BuildPool2d(MLGraphBuilder* builder,
                       webnn::mojom::blink::Pool2d::Kind kind,
                       const MLOperand* input,
                       const MLPool2dOptions* options,
                       ExceptionState& exception_state) {
  auto pool2d_attributes = ConvertToPool2dAttributes(options);
  if (!pool2d_attributes.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(pool2d_attributes.error()));
    return nullptr;
  }

  auto validated_output = webnn::ValidatePool2dAndInferOutput(
      ConvertToComponentOperand(input), std::move(pool2d_attributes.value()));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  // Create pool2d operator and its output operand. Connect the pool2d operator
  // to its input and output operands.
  auto* pool2d = MakeGarbageCollected<MLOperator>(
      builder, /*kind=*/webnn::mojom::blink::Operation::Tag::kPool2d,
      /*sub_kind=*/kind, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      builder, input->DataType(),
      Vector<uint32_t>(validated_output->dimensions), pool2d);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  pool2d->Connect({input}, {output.value()});
  return output.value();
}

}  // namespace

// static
MLGraphBuilder* MLGraphBuilder::Create(MLContext* context) {
  return MakeGarbageCollected<MLGraphBuilder>(context);
}

MLGraphBuilder::MLGraphBuilder(MLContext* context) : ml_context_(context) {}

MLGraphBuilder::~MLGraphBuilder() = default;

void MLGraphBuilder::Trace(Visitor* visitor) const {
  visitor->Trace(ml_context_);
  ScriptWrappable::Trace(visitor);
}

MLContext* MLGraphBuilder::GetContext() const {
  return ml_context_.Get();
}

MLOperand* MLGraphBuilder::input(String name,
                                 const MLOperandDescriptor* desc,
                                 ExceptionState& exception_state) {
  auto input_operand = MLOperand::ValidateAndCreateInput(
      this, desc->dataType().AsEnum(), desc->dimensions(), std::move(name));
  if (!input_operand.has_value()) {
    exception_state.ThrowTypeError(input_operand.error());
    return nullptr;
  }
  return input_operand.value();
}

MLOperand* MLGraphBuilder::constant(const MLOperandDescriptor* desc,
                                    NotShared<DOMArrayBufferView> buffer_view,
                                    ExceptionState& exception_state) {
  auto constant_operand = MLOperand::ValidateAndCreateConstant(
      this, desc->dataType().AsEnum(), desc->dimensions(), buffer_view.Get());
  if (!constant_operand.has_value()) {
    exception_state.ThrowTypeError(constant_operand.error());
    return nullptr;
  }
  return constant_operand.value();
}

MLOperand* MLGraphBuilder::argMin(const MLOperand* input,
                                  const MLArgMinMaxOptions* options,
                                  ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);
  return BuildArgMinMax(this, webnn::mojom::blink::ArgMinMax::Kind::kMin, input,
                        options, exception_state);
}

MLOperand* MLGraphBuilder::argMax(const MLOperand* input,
                                  const MLArgMinMaxOptions* options,
                                  ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);
  return BuildArgMinMax(this, webnn::mojom::blink::ArgMinMax::Kind::kMax, input,
                        options, exception_state);
}

MLOperand* MLGraphBuilder::batchNormalization(
    const MLOperand* input,
    const MLOperand* mean,
    const MLOperand* variance,
    const MLBatchNormalizationOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, mean, variance};
  // Adding the optional operands into inputs ensures the graph traversal
  // algorithm GetOperatorsInTopologicalOrder() works. For backends, the
  // optional operands should be retrieved from the options instead of inputs.
  if (options->hasScale()) {
    inputs.push_back(options->scale());
  }
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  if (options->hasActivation()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivation(options->activation()),
                                   nullptr);
  }

  const auto validated_output = webnn::ValidateBatchNormalizationAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(mean),
      ConvertToComponentOperand(variance),
      ConvertToBatchNormalizationAttributes(options));
  if (!validated_output.has_value()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kDataError,
        WTF::String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  // Create batchNormalization operator and its output operand. Connect the
  // batchNormalization operator to its input and output operands.
  auto* batch_normalization = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kBatchNormalization,
      /*sub_kind=*/absl::monostate{}, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), batch_normalization);
  if (!output.has_value()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kDataError,
                                      output.error());
    return nullptr;
  }
  batch_normalization->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::concat(const HeapVector<Member<MLOperand>>& inputs,
                                  const uint32_t axis,
                                  ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(
      ValidateInputs(static_cast<HeapVector<Member<const MLOperand>>>(inputs)),
      nullptr);

  std::vector<webnn::Operand> input_component_operands;
  input_component_operands.reserve(inputs.size());
  base::ranges::transform(
      inputs, std::back_inserter(input_component_operands),
      [](const auto& input) { return ConvertToComponentOperand(input); });

  auto validated_output =
      webnn::ValidateConcatAndInferOutput(input_component_operands, axis);
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* concat = MakeGarbageCollected<MLConcatOperator>(this, axis);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), concat);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  concat->Connect(static_cast<HeapVector<Member<const MLOperand>>>(inputs),
                  {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::clamp(const MLOperand* input,
                                 const MLClampOptions* options,
                                 ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  if (!ValidateClampOptions(options, exception_state)) {
    return nullptr;
  }
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-clamp, the output tensor of
  // clamp has the same data type and dimensions as its input.
  return BuildUnaryOperator(
      this, exception_state, webnn::mojom::blink::Operation::Tag::kClamp,
      webnn::DataTypeConstraintSet::All(), input, options);
}

MLActivation* MLGraphBuilder::clamp(const MLClampOptions* options,
                                    ExceptionState& exception_state) {
  if (!ValidateClampOptions(options, exception_state)) {
    return nullptr;
  }
  // Create the clamp operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kClamp, options);
}

MLOperand* MLGraphBuilder::conv2d(const MLOperand* input,
                                  const MLOperand* filter,
                                  const MLConv2dOptions* options,
                                  ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, filter};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  if (options->hasActivation()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivation(options->activation()),
                                   nullptr);
  }

  auto conv2d_attributes = ConvertToConv2dAttributes(options);
  if (!conv2d_attributes.has_value()) {
    exception_state.ThrowTypeError(conv2d_attributes.error());
    return nullptr;
  }

  auto validated_output = webnn::ValidateConv2dAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(filter),
      std::move(conv2d_attributes.value()));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  // Create conv2d operator and its output operand. Connect the conv2d operator
  // to its input and output operands.
  auto* conv2d = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kConv2d,
      /*sub_type=*/webnn::mojom::blink::Conv2d::Kind::kDirect, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output.value().data_type),
      Vector<uint32_t>(validated_output.value().dimensions), conv2d);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  conv2d->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::convTranspose2d(
    const MLOperand* input,
    const MLOperand* filter,
    const MLConvTranspose2dOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, filter};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  if (options->hasActivation()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivation(options->activation()),
                                   nullptr);
  }

  auto convTranspose2d_attributes = ConvertToConvTranspose2dAttributes(options);
  if (!convTranspose2d_attributes.has_value()) {
    exception_state.ThrowTypeError(convTranspose2d_attributes.error());
    return nullptr;
  }

  auto validated_output = webnn::ValidateConvTranspose2dAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(filter),
      std::move(convTranspose2d_attributes.value()));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  // Create convTranspose2d operator and its output operand. Connect the
  // convTranspose2d operator to its input and output operands.
  auto* convTranspose2d = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kConv2d,
      /*sub_type=*/webnn::mojom::blink::Conv2d::Kind::kTransposed, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output.value().data_type),
      Vector<uint32_t>(validated_output.value().dimensions), convTranspose2d);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  convTranspose2d->Connect(std::move(inputs), {output.value()});
  return output.value();
}

#define BUILD_ELEMENTWISE_BINARY_OP(op, op_kind)                           \
  MLOperand* MLGraphBuilder::op(const MLOperand* a, const MLOperand* b,    \
                                ExceptionState& exception_state) {         \
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs({a, b}), nullptr);       \
    return BuildElementWiseBinary(                                         \
        this, webnn::mojom::blink::ElementWiseBinary::Kind::op_kind, a, b, \
        exception_state);                                                  \
  }

BUILD_ELEMENTWISE_BINARY_OP(add, kAdd)
BUILD_ELEMENTWISE_BINARY_OP(sub, kSub)
BUILD_ELEMENTWISE_BINARY_OP(mul, kMul)
BUILD_ELEMENTWISE_BINARY_OP(div, kDiv)
BUILD_ELEMENTWISE_BINARY_OP(min, kMin)
BUILD_ELEMENTWISE_BINARY_OP(max, kMax)
BUILD_ELEMENTWISE_BINARY_OP(pow, kPow)
BUILD_ELEMENTWISE_BINARY_OP(equal, kEqual)
BUILD_ELEMENTWISE_BINARY_OP(greater, kGreater)
BUILD_ELEMENTWISE_BINARY_OP(greaterOrEqual, kGreaterOrEqual)
BUILD_ELEMENTWISE_BINARY_OP(lesser, kLesser)
BUILD_ELEMENTWISE_BINARY_OP(lesserOrEqual, kLesserOrEqual)

#define BUILD_ELEMENTWISE_UNARY_OP(op, op_kind, data_type_constraint) \
  MLOperand* MLGraphBuilder::op(const MLOperand* input,               \
                                ExceptionState& exception_state) {    \
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);    \
    return BuildElementWiseUnaryOperator(                             \
        this, exception_state,                                        \
        webnn::mojom::blink::ElementWiseUnary::Kind::op_kind,         \
        data_type_constraint, input);                                 \
  }

BUILD_ELEMENTWISE_UNARY_OP(abs,
                           kAbs,
                           webnn::DataTypeConstraint::kFloat16To32Int8To32)
BUILD_ELEMENTWISE_UNARY_OP(ceil, kCeil, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(cos, kCos, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(exp, kExp, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(floor, kFloor, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(log, kLog, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(neg,
                           kNeg,
                           webnn::DataTypeConstraint::kFloat16To32Int8To32)
BUILD_ELEMENTWISE_UNARY_OP(sin, kSin, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(tan, kTan, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(erf, kErf, webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(identity,
                           kIdentity,
                           webnn::DataTypeConstraintSet::All())
BUILD_ELEMENTWISE_UNARY_OP(logicalNot,
                           kLogicalNot,
                           {webnn::Operand::DataType::kUint8})
BUILD_ELEMENTWISE_UNARY_OP(reciprocal,
                           kReciprocal,
                           webnn::DataTypeConstraint::kFloat)
BUILD_ELEMENTWISE_UNARY_OP(sqrt, kSqrt, webnn::DataTypeConstraint::kFloat)

MLOperand* MLGraphBuilder::cast(const MLOperand* input,
                                const V8MLOperandDataType output_data_type,
                                ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  auto* cast = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kElementWiseUnary,
      /*sub_kind=*/webnn::mojom::blink::ElementWiseUnary::Kind::kCast);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, output_data_type.AsEnum(), input->Dimensions(), cast);
  if (!output.has_value()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kDataError,
                                      output.error());
    return nullptr;
  }
  cast->Connect({input}, {output.value()});
  return output.value();
}

#define BUILD_REDUCE_OP(op, op_kind)                                     \
  MLOperand* MLGraphBuilder::op(const MLOperand* input,                  \
                                const MLReduceOptions* options,          \
                                ExceptionState& exception_state) {       \
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);       \
    return BuildReduce(this, webnn::mojom::blink::Reduce::Kind::op_kind, \
                       input, options, exception_state);                 \
  }

BUILD_REDUCE_OP(reduceL1, kL1)
BUILD_REDUCE_OP(reduceL2, kL2)
BUILD_REDUCE_OP(reduceLogSum, kLogSum)
BUILD_REDUCE_OP(reduceLogSumExp, kLogSumExp)
BUILD_REDUCE_OP(reduceMax, kMax)
BUILD_REDUCE_OP(reduceMean, kMean)
BUILD_REDUCE_OP(reduceMin, kMin)
BUILD_REDUCE_OP(reduceProduct, kProduct)
BUILD_REDUCE_OP(reduceSum, kSum)
BUILD_REDUCE_OP(reduceSumSquare, kSumSquare)

MLOperand* MLGraphBuilder::elu(const MLOperand* input,
                               const MLEluOptions* options,
                               ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The current spec doesn't restrict the value of alpha. An issue has been
  // filed to track it: https://github.com/webmachinelearning/webnn/issues/383
  if (options->alpha() <= 0.0f) {
    exception_state.ThrowTypeError(
        "The value of alpha must be greater than 0.");
    return nullptr;
  }
  // The current spec doesn't specify the operand data type constraints of elu.
  // An issue has been filed to track it:
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-elu, the output tensor of
  // elu has the same data type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kElu,
                            webnn::DataTypeConstraint::kFloat, input, options);
}

MLActivation* MLGraphBuilder::elu(const MLEluOptions* options,
                                  ExceptionState& exception_state) {
  // The current spec doesn't restrict the value of alpha. An issue has been
  // filed to track it: https://github.com/webmachinelearning/webnn/issues/383
  if (options->alpha() <= 0.0f) {
    exception_state.ThrowTypeError(
        "The value of alpha must be greater than 0.");
    return nullptr;
  }
  // Create the elu operator that would be used as an activation
  // function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kElu, options);
}

MLOperand* MLGraphBuilder::expand(const MLOperand* input,
                                  const Vector<uint32_t>& new_shape,
                                  ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  auto output_shape = BroadcastShapes(input->Dimensions(), new_shape, false);
  if (!output_shape) {
    exception_state.ThrowTypeError(
        "The input shape is not broadcastable to the new shape.");
    return nullptr;
  }
  CHECK(output_shape.value() == new_shape);

  auto* expand = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kExpand);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, input->DataType(), output_shape.value(), expand);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  expand->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::gather(const MLOperand* input,
                                  const MLOperand* indices,
                                  const MLGatherOptions* options,
                                  ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, indices};
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  auto validated_output = webnn::ValidateGatherAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(indices),
      options->axis());
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* gather = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kGather,
      /*sub_kind=*/absl::monostate{}, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), gather);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }

  gather->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::gelu(const MLOperand* input,
                                ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-gelu, the output tensor of
  // gelu has the same data type and dimensions as its input. And the input data
  // type must be one of the floating point types.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kGelu,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::gelu(ExceptionState& exception_state) {
  // Create the gelu operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kGelu);
}

MLOperand* MLGraphBuilder::gemm(const MLOperand* a,
                                const MLOperand* b,
                                const MLGemmOptions* options,
                                ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {a, b};
  if (options->hasC()) {
    inputs.push_back(options->c());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  auto validated_output = webnn::ValidateGemmAndInferOutput(
      ConvertToComponentOperand(a), ConvertToComponentOperand(b),
      ConvertToGemmAttributes(options));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  auto* gemm = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kGemm,
      /*sub_kind=*/absl::monostate{}, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output.value().data_type),
      Vector<uint32_t>(validated_output.value().dimensions), gemm);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  gemm->Connect(std::move(inputs), {output.value()});
  return output.value();
}

HeapVector<Member<const MLOperand>> MLGraphBuilder::gru(
    const MLOperand* input,
    const MLOperand* weight,
    const MLOperand* recurrent_weight,
    const uint32_t steps,
    const uint32_t hidden_size,
    MLGruOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, weight,
                                                recurrent_weight};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  if (options->hasRecurrentBias()) {
    inputs.push_back(options->recurrentBias());
  }
  if (options->hasInitialHiddenState()) {
    inputs.push_back(options->initialHiddenState());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs),
                                 HeapVector<Member<const MLOperand>>());

  if (options->hasActivations()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivations(options->activations()),
                                   HeapVector<Member<const MLOperand>>());
  }

  auto validated_outputs = webnn::ValidateGruAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(weight),
      ConvertToComponentOperand(recurrent_weight), steps, hidden_size,
      ConvertToGruAttributes(this, options));
  if (!validated_outputs.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_outputs.error()));
    return {};
  }
  auto* gru =
      MakeGarbageCollected<MLGruOperator>(this, steps, hidden_size, options);

  HeapVector<Member<const MLOperand>> outputs;
  for (const auto& validated_output : validated_outputs.value()) {
    auto output = MLOperand::ValidateAndCreateOutput(
        this, ComponentOperandTypeToBlink(validated_output.data_type),
        Vector<uint32_t>(validated_output.dimensions), gru);
    if (!output.has_value()) {
      exception_state.ThrowTypeError(output.error());
      return {};
    }
    outputs.push_back(output.value());
  }

  gru->Connect(std::move(inputs), outputs);
  return outputs;
}

MLOperand* MLGraphBuilder::gruCell(const MLOperand* input,
                                   const MLOperand* weight,
                                   const MLOperand* recurrent_weight,
                                   const MLOperand* hidden_state,
                                   const uint32_t hidden_size,
                                   MLGruCellOptions* options,
                                   ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, weight, recurrent_weight,
                                                hidden_state};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  if (options->hasRecurrentBias()) {
    inputs.push_back(options->recurrentBias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  if (options->hasActivations()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivations(options->activations()),
                                   nullptr);
  }

  auto validated_output = webnn::ValidateGruCellAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(weight),
      ConvertToComponentOperand(recurrent_weight),
      ConvertToComponentOperand(hidden_state), hidden_size,
      ConvertToGruCellAttributes(this, options));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return {};
  }
  auto* gru_cell =
      MakeGarbageCollected<MLGruCellOperator>(this, hidden_size, options);

  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), gru_cell);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return {};
  }

  gru_cell->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::hardSwish(const MLOperand* input,
                                     ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The input data type must be one of the floating point types. Although this
  // constraint is not specified in current WebNN spec, there is a feature
  // request for that: https://github.com/webmachinelearning/webnn/issues/283
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-hard-swish, the output
  // tensor of hard-swish has the same data type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kHardSwish,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::hardSwish(ExceptionState& exception_state) {
  // TODO: crbug.com/40206287 - Support HardSwish as an activation function.
  NOTIMPLEMENTED();
  return nullptr;
}

MLOperand* MLGraphBuilder::hardSigmoid(const MLOperand* input,
                                       const MLHardSigmoidOptions* options,
                                       ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The current spec doesn't specify the operand data type constraints of
  // hardSigmoid. An issue has been filed to track it:
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-hardsigmoid, the output
  // tensor of softplus has the same type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kHardSigmoid,
                            webnn::DataTypeConstraint::kFloat, input, options);
}

MLActivation* MLGraphBuilder::hardSigmoid(const MLHardSigmoidOptions* options,
                                          ExceptionState& exception_state) {
  // Create the hardSigmoid operator that would be used as an activation
  // function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kHardSigmoid, options);
}

MLOperand* MLGraphBuilder::instanceNormalization(
    const MLOperand* input,
    const MLInstanceNormalizationOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input};
  // Adding the optional operands into inputs ensures the graph traversal
  // algorithm GetOperatorsInTopologicalOrder() works. For backends, the
  // optional operands should be retrieved from the options instead of inputs.
  if (options->hasScale()) {
    inputs.push_back(options->scale());
  }
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  const auto validated_output =
      webnn::ValidateInstanceNormalizationAndInferOutput(
          ConvertToComponentOperand(input),
          ConvertToInstanceNormalizationAttributes(options));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* instance_normalization = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kInstanceNormalization,
      /*sub_kind=*/absl::monostate{}, options);

  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), instance_normalization);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }

  instance_normalization->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::layerNormalization(
    const MLOperand* input,
    const MLLayerNormalizationOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input};
  // Adding the optional operands into inputs ensures the graph traversal
  // algorithm GetOperatorsInTopologicalOrder() works. For backends, the
  // optional operands should be retrieved from the options instead of inputs.
  if (options->hasScale()) {
    inputs.push_back(options->scale());
  }
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  // TODO(crbug.com/1273291): Figure out whether the `axes` should be required,
  // tracked by issue: https://github.com/webmachinelearning/webnn/issues/487
  const Vector<uint32_t> axes = options->getAxesOr(
      CreateLayerNormalizationDefaultAxes(input->Dimensions().size()));

  const auto validated_output = webnn::ValidateLayerNormalizationAndInferOutput(
      ConvertToComponentOperand(input), axes,
      ConvertToLayerNormalizationAttributes(options));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* layer_normalization = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kLayerNormalization,
      /*sub_kind=*/absl::monostate{}, options);

  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), layer_normalization);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }

  layer_normalization->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::leakyRelu(const MLOperand* input,
                                     const MLLeakyReluOptions* options,
                                     ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The current spec doesn't specify the operand data type constraints of
  // leakyRelu. An issue has been filed to track it:
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-leakyrelu, the output
  // tensor of leaky relu has the same type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kLeakyRelu,
                            webnn::DataTypeConstraint::kFloat, input, options);
}

MLActivation* MLGraphBuilder::leakyRelu(const MLLeakyReluOptions* options,
                                        ExceptionState& exception_state) {
  // Create the leakyRelu operator that would be used as an activation
  // function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kLeakyRelu, options);
}

MLOperand* MLGraphBuilder::linear(const MLOperand* input,
                                  const MLLinearOptions* options,
                                  ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The current spec doesn't specify the operand data type constraints of
  // linear. An issue has been filed to track it:
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-linear, the output tensor
  // of linear has the same type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kLinear,
                            webnn::DataTypeConstraint::kFloat, input, options);
}

MLActivation* MLGraphBuilder::linear(const MLLinearOptions* options,
                                     ExceptionState& exception_state) {
  // Create the linear operator that would be used as an activation
  // function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kLinear, options);
}

HeapVector<Member<const MLOperand>> MLGraphBuilder::lstm(
    const MLOperand* input,
    const MLOperand* weight,
    const MLOperand* recurrent_weight,
    const uint32_t steps,
    const uint32_t hidden_size,
    MLLstmOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, weight,
                                                recurrent_weight};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  if (options->hasRecurrentBias()) {
    inputs.push_back(options->recurrentBias());
  }
  if (options->hasPeepholeWeight()) {
    inputs.push_back(options->peepholeWeight());
  }
  if (options->hasInitialHiddenState()) {
    inputs.push_back(options->initialHiddenState());
  }
  if (options->hasInitialCellState()) {
    inputs.push_back(options->initialCellState());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs),
                                 HeapVector<Member<const MLOperand>>());

  if (options->hasActivations()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivations(options->activations()),
                                   HeapVector<Member<const MLOperand>>());
  }

  // If the activations are not specified, create a default activation sequence
  // [sigmoid, tanh, tanh] as defined in the spec.
  if (!options->hasActivations()) {
    MLActivation* activation_sigmoid = MakeGarbageCollected<MLActivation>(
        this, webnn::mojom::blink::Activation::Tag::kSigmoid);
    MLActivation* activation_tanh = MakeGarbageCollected<MLActivation>(
        this, webnn::mojom::blink::Activation::Tag::kTanh);
    options->setActivations(
        {activation_sigmoid, activation_tanh, activation_tanh});
  }

  auto validated_outputs = webnn::ValidateLstmAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(weight),
      ConvertToComponentOperand(recurrent_weight), steps, hidden_size,
      ConvertToLstmAttributes(options));
  if (!validated_outputs.has_value()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kDataError,
        String::FromUTF8(validated_outputs.error()));
    return {};
  }

  auto* lstm =
      MakeGarbageCollected<MLLstmOperator>(this, steps, hidden_size, options);

  HeapVector<Member<const MLOperand>> outputs;
  for (const auto& validated_output : validated_outputs.value()) {
    auto output = MLOperand::ValidateAndCreateOutput(
        this, ComponentOperandTypeToBlink(validated_output.data_type),
        Vector<uint32_t>(validated_output.dimensions), lstm);
    if (!output.has_value()) {
      exception_state.ThrowDOMException(DOMExceptionCode::kDataError,
                                        output.error());
      return {};
    }
    outputs.push_back(output.value());
  }

  lstm->Connect(std::move(inputs), outputs);
  return outputs;
}

HeapVector<Member<const MLOperand>> MLGraphBuilder::lstmCell(
    const MLOperand* input,
    const MLOperand* weight,
    const MLOperand* recurrent_weight,
    const MLOperand* hidden_state,
    const MLOperand* cell_state,
    const uint32_t hidden_size,
    MLLstmCellOptions* options,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, weight, recurrent_weight,
                                                hidden_state, cell_state};
  if (options->hasBias()) {
    inputs.push_back(options->bias());
  }
  if (options->hasRecurrentBias()) {
    inputs.push_back(options->recurrentBias());
  }
  if (options->hasPeepholeWeight()) {
    inputs.push_back(options->peepholeWeight());
  }
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs),
                                 HeapVector<Member<const MLOperand>>());

  if (options->hasActivations()) {
    THROW_AND_RETURN_TYPE_IF_ERROR(ValidateActivations(options->activations()),
                                   HeapVector<Member<const MLOperand>>());
  }

  // If the activations are not specified, create a default activation sequence
  // [sigmoid, tanh, tanh] as defined in the spec.
  if (!options->hasActivations()) {
    MLActivation* activation_sigmoid = MakeGarbageCollected<MLActivation>(
        this, webnn::mojom::blink::Activation::Tag::kSigmoid);
    MLActivation* activation_tanh = MakeGarbageCollected<MLActivation>(
        this, webnn::mojom::blink::Activation::Tag::kTanh);
    options->setActivations(
        {activation_sigmoid, activation_tanh, activation_tanh});
  }

  auto validated_outputs = webnn::ValidateLstmCellAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(weight),
      ConvertToComponentOperand(recurrent_weight),
      ConvertToComponentOperand(hidden_state),
      ConvertToComponentOperand(cell_state), hidden_size,
      ConvertToLstmCellAttributes(options));
  if (!validated_outputs.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_outputs.error()));
    return {};
  }

  auto* lstm_cell =
      MakeGarbageCollected<MLLstmCellOperator>(this, hidden_size, options);

  HeapVector<Member<const MLOperand>> outputs;
  CHECK_EQ(validated_outputs->size(), 2u);
  outputs.reserve(2);
  for (const webnn::Operand& validated_output : validated_outputs.value()) {
    auto output = MLOperand::ValidateAndCreateOutput(
        this, ComponentOperandTypeToBlink(validated_output.data_type),
        Vector<uint32_t>(validated_output.dimensions), lstm_cell);
    if (!output.has_value()) {
      exception_state.ThrowTypeError(output.error());
      return {};
    }
    outputs.push_back(output.value());
  }

  lstm_cell->Connect(std::move(inputs), outputs);
  return outputs;
}

MLOperand* MLGraphBuilder::matmul(const MLOperand* a,
                                  const MLOperand* b,
                                  ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {a, b};
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  auto validated_output = webnn::ValidateMatmulAndInferOutput(
      ConvertToComponentOperand(a), ConvertToComponentOperand(b));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(
        WTF::String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  // Create matmul operator and its output operand. Connect the matmul operator
  // to its input and output operands.
  auto* matmul = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kMatmul);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output.value().data_type),
      Vector<uint32_t>(validated_output.value().dimensions), matmul);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  matmul->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::pad(const MLOperand* input,
                               const Vector<uint32_t>& beginning_padding,
                               const Vector<uint32_t>& ending_padding,
                               const MLPadOptions* options,
                               ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  auto validated_output = webnn::ValidatePadAndInferOutput(
      ConvertToComponentOperand(input), beginning_padding, ending_padding);
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  if (options->mode().AsEnum() != V8MLPaddingMode::Enum::kConstant &&
      fabs(options->value() - 0.0f) > std::numeric_limits<float>::epsilon()) {
    ml_context_->LogConsoleWarning(
        "The pad value is ignored unless the options.mode is set to "
        "constant.");
  }

  auto* pad = MakeGarbageCollected<MLPadOperator>(this, beginning_padding,
                                                  ending_padding, options);
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-pad, the output
  // tensor of pad has the same data type as its input.
  auto output = MLOperand::ValidateAndCreateOutput(
      this, input->DataType(), Vector<uint32_t>(validated_output->dimensions),
      pad);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  pad->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::averagePool2d(const MLOperand* input,
                                         const MLPool2dOptions* options,
                                         ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  if (!(input->DataType() == V8MLOperandDataType::Enum::kFloat32 ||
        input->DataType() == V8MLOperandDataType::Enum::kFloat16)) {
    exception_state.ThrowTypeError(
        "The input data type must be a floating point type.");
    return nullptr;
  }

  return BuildPool2d(this, webnn::mojom::blink::Pool2d::Kind::kAveragePool2d,
                     input, options, exception_state);
}

MLOperand* MLGraphBuilder::l2Pool2d(const MLOperand* input,
                                    const MLPool2dOptions* options,
                                    ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  if (!(input->DataType() == V8MLOperandDataType::Enum::kFloat32 ||
        input->DataType() == V8MLOperandDataType::Enum::kFloat16)) {
    exception_state.ThrowTypeError(
        "The input data type must be a floating point type.");
    return nullptr;
  }

  return BuildPool2d(this, webnn::mojom::blink::Pool2d::Kind::kL2Pool2d, input,
                     options, exception_state);
}

MLOperand* MLGraphBuilder::maxPool2d(const MLOperand* input,
                                     const MLPool2dOptions* options,
                                     ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  return BuildPool2d(this, webnn::mojom::blink::Pool2d::Kind::kMaxPool2d, input,
                     options, exception_state);
}

MLOperand* MLGraphBuilder::prelu(const MLOperand* input,
                                 const MLOperand* slope,
                                 ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {input, slope};
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  auto validated_output = webnn::ValidatePreluAndInferOutput(
      ConvertToComponentOperand(input), ConvertToComponentOperand(slope));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* prelu = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kPrelu,
      /*sub_kind=*/absl::monostate{});
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), prelu);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  prelu->Connect(std::move(inputs), {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::relu(const MLOperand* input,
                                ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-relu, the output tensor of
  // relu has the same data type and dimensions as its input.
  return BuildUnaryOperator(
      this, exception_state, webnn::mojom::blink::Operation::Tag::kRelu,
      webnn::DataTypeConstraint::kFloat16To32Int8To32, input);
}

MLActivation* MLGraphBuilder::relu(ExceptionState& exception_state) {
  // Create the relu operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kRelu);
}

MLOperand* MLGraphBuilder::reshape(const MLOperand* input,
                                   const Vector<uint32_t>& new_shape,
                                   ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // Setting the initial number of elements to 1 would cover the 0-D scalar with
  // empty dimensions.
  base::CheckedNumeric<size_t> checked_newshape_number_of_elements = 1;
  Vector<uint32_t> output_shape(new_shape.size());
  for (wtf_size_t i = 0; i < new_shape.size(); ++i) {
    auto dim = new_shape[i];
    if (dim == 0) {
      exception_state.ThrowTypeError("The value of new shape should not be 0.");
      return nullptr;
    }
    checked_newshape_number_of_elements *= dim;
    output_shape[i] = dim;
  }
  size_t newshape_number_of_elements;
  if (!checked_newshape_number_of_elements.AssignIfValid(
          &newshape_number_of_elements)) {
    exception_state.ThrowTypeError(
        "The number of elements implied by new shape is too large.");
    return nullptr;
  }
  DCHECK_NE(newshape_number_of_elements, size_t(0));
  // The number of elements implied by new shape must be the same as the
  // number of elements in the input tensor.
  if (input->NumberOfElements() != newshape_number_of_elements) {
    exception_state.ThrowTypeError(String::Format(
        "The number of elements (%zu) implied by new shape doesn't match "
        "the number of elements (%zu) in the input tensor.",
        newshape_number_of_elements, input->NumberOfElements()));
    return nullptr;
  }
  auto* reshape = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kReshape);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, input->DataType(), std::move(output_shape), reshape);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  reshape->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::resample2d(const MLOperand* input,
                                      const MLResample2dOptions* options,
                                      ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  absl::variant<base::span<const float>, base::span<const uint32_t>>
      scales_or_sizes;
  Vector<float> default_scales = {1.0, 1.0};
  if (options->hasSizes()) {
    if (options->hasScales()) {
      ml_context_->LogConsoleWarning(
          "When sizes and scales are both specified, scales argument is "
          "ignored.");
    }
    scales_or_sizes = options->sizes();
  } else {
    scales_or_sizes = options->hasScales() ? options->scales() : default_scales;
  }

  auto validated_output = webnn::ValidateResample2dAndInferOutput(
      ConvertToComponentOperand(input), scales_or_sizes,
      options->getAxesOr({2, 3}));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  // Create resample2d operator and its output operand. Connect the resample2d
  // operator to its input and output operands.
  auto* resample2d = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kResample2d,
      /*sub_kind=*/absl::monostate{}, options);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), resample2d);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  resample2d->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::sigmoid(const MLOperand* input,
                                   ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // According to WebNN spec
  // https://webmachinelearning.github.io/webnn/#api-mlgraphbuilder-sigmoid, the
  // output tensor of sigmoid has the same data type and dimensions as its
  // input. And the input data type must be one of the floating point types.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kSigmoid,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::sigmoid(ExceptionState& exception_state) {
  // Create the sigmoid operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kSigmoid);
}

MLOperand* MLGraphBuilder::slice(const MLOperand* input,
                                 const Vector<uint32_t>& starts,
                                 const Vector<uint32_t>& sizes,
                                 ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  webnn::SliceAttributes attributes;
  attributes.sizes.assign(sizes.begin(), sizes.end());
  attributes.starts.assign(starts.begin(), starts.end());
  auto validated_output = webnn::ValidateSliceAndInferOutput(
      ConvertToComponentOperand(input), attributes);
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* slice = MakeGarbageCollected<MLSliceOperator>(this, starts, sizes);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), slice);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  slice->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::softmax(const MLOperand* input,
                                   ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  auto validated_output =
      webnn::ValidateSoftmaxAndInferOutput(ConvertToComponentOperand(input));
  if (!validated_output.has_value()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kDataError,
        WTF::String::FromUTF8(validated_output.error()));
    return nullptr;
  }
  auto* softmax = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kSoftmax);
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output.value().data_type),
      Vector<uint32_t>(validated_output.value().dimensions), softmax);
  if (!output.has_value()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kDataError,
                                      output.error());
    return nullptr;
  }
  softmax->Connect({input}, {output.value()});
  return output.value();
}

MLActivation* MLGraphBuilder::softmax(ExceptionState& exception_state) {
  // Create the softmax operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kSoftmax);
}

MLOperand* MLGraphBuilder::softplus(const MLOperand* input,
                                    ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The current spec doesn't specify the operand data type constraints of
  // softplus. An issue has been filed to track it:
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-softplus, the output
  // tensor of softplus has the same type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kSoftplus,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::softplus(ExceptionState& exception_state) {
  // Create the softplus operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kSoftplus);
}

MLOperand* MLGraphBuilder::softsign(const MLOperand* input,
                                    ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The input data type must be one of the floating point types.
  // The current spec doesn't specify the operand data type constraints of
  // softsign, an issue has been filed to track it-
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-softsign, the output tensor
  // of softsign has the same data type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kSoftsign,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::softsign(ExceptionState& exception_state) {
  // Create the softsign operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kSoftsign);
}

HeapVector<Member<const MLOperand>> MLGraphBuilder::split(
    const MLOperand* input,
    const uint32_t splits,
    const MLSplitOptions* options,
    ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input),
                                 HeapVector<Member<const MLOperand>>());

  auto validated_outputs = webnn::ValidateSplitAndInferOutput(
      ConvertToComponentOperand(input), {
                                            .splits = splits,
                                            .axis = options->axis(),
                                        });
  if (!validated_outputs.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_outputs.error()));
    return {};
  }

  auto* split = MakeGarbageCollected<MLSplitOperator>(this, splits, options);
  HeapVector<Member<const MLOperand>> outputs;
  for (const auto& validated_output : validated_outputs.value()) {
    auto output = MLOperand::ValidateAndCreateOutput(
        this, ComponentOperandTypeToBlink(validated_output.data_type),
        Vector<uint32_t>(validated_output.dimensions), split);
    if (!output.has_value()) {
      exception_state.ThrowTypeError(output.error());
      return {};
    }
    outputs.push_back(output.value());
  }
  split->Connect({input}, outputs);
  return outputs;
}

// There are some backends don't support "split into sizes" variant, e.g.
// XNNPACK, and there is an ongoing discussion in WG:
// https://github.com/webmachinelearning/webnn/issues/392
HeapVector<Member<const MLOperand>> MLGraphBuilder::split(
    const MLOperand* input,
    const Vector<uint32_t>& splits,
    const MLSplitOptions* options,
    ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input),
                                 HeapVector<Member<const MLOperand>>());

  auto validated_outputs = webnn::ValidateSplitAndInferOutput(
      ConvertToComponentOperand(input), {
                                            .splits = splits,
                                            .axis = options->axis(),
                                        });
  if (!validated_outputs.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_outputs.error()));
    return {};
  }

  auto* split = MakeGarbageCollected<MLSplitOperator>(this, splits, options);
  HeapVector<Member<const MLOperand>> outputs;
  for (const auto& validated_output : validated_outputs.value()) {
    auto output = MLOperand::ValidateAndCreateOutput(
        this, ComponentOperandTypeToBlink(validated_output.data_type),
        Vector<uint32_t>(validated_output.dimensions), split);
    if (!output.has_value()) {
      exception_state.ThrowTypeError(output.error());
      return {};
    }
    outputs.push_back(output.value());
  }
  split->Connect({input}, outputs);
  return outputs;
}

MLOperand* MLGraphBuilder::tanh(const MLOperand* input,
                                ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // The input data type must be one of the floating point types.
  // The current spec doesn't specify the operand data type constraints of tanh,
  // an issue has been filed to track it-
  // https://github.com/webmachinelearning/webnn/issues/283.
  //
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-tanh, the output tensor of
  // tanh has the same data type and dimensions as its input.
  return BuildUnaryOperator(this, exception_state,
                            webnn::mojom::blink::Operation::Tag::kTanh,
                            webnn::DataTypeConstraint::kFloat, input);
}

MLActivation* MLGraphBuilder::tanh(ExceptionState& exception_state) {
  // Create the tanh operator that would be used as an activation function.
  return MakeGarbageCollected<MLActivation>(
      this, webnn::mojom::blink::Activation::Tag::kTanh);
}

MLOperand* MLGraphBuilder::transpose(const MLOperand* input,
                                     const MLTransposeOptions* options,
                                     ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  // According to WebNN spec:
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-transpose,
  // When permutation is not specified, it’s set to [N-1, ..., 0], where N is
  // the rank of the input tensor.
  auto input_rank = input->Dimensions().size();
  const Vector<uint32_t> permutation =
      options->getPermutationOr(CreateDefaultPermutation(input_rank));
  auto validated_output = webnn::ValidateTransposeAndInferOutput(
      ConvertToComponentOperand(input), permutation);
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* transpose = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kTranspose,
      /*sub_kind=*/absl::monostate{}, options);
  // According to WebNN spec
  // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-transpose, the output
  // tensor of transpose has the same data type as its input.
  auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), transpose);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  transpose->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::triangular(const MLOperand* input,
                                      const MLTriangularOptions* options,
                                      ExceptionState& exception_state) {
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInput(input), nullptr);

  const base::expected<webnn::Operand, std::string> validated_output =
      webnn::ValidateTriangularAndInferOutput(ConvertToComponentOperand(input));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* triangular = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kTriangular,
      /*sub_kind=*/absl::monostate{}, options);
  const base::expected<MLOperand*, String> output =
      MLOperand::ValidateAndCreateOutput(
          this, ComponentOperandTypeToBlink(validated_output->data_type),
          Vector<uint32_t>(validated_output->dimensions), triangular);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  triangular->Connect({input}, {output.value()});
  return output.value();
}

MLOperand* MLGraphBuilder::where(const MLOperand* condition,
                                 const MLOperand* true_value,
                                 const MLOperand* false_value,
                                 ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> inputs = {condition, true_value,
                                                false_value};
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(inputs), nullptr);

  const auto validated_output = webnn::ValidateWhereAndInferOutput(
      ConvertToComponentOperand(condition),
      ConvertToComponentOperand(true_value),
      ConvertToComponentOperand(false_value));
  if (!validated_output.has_value()) {
    exception_state.ThrowTypeError(String::FromUTF8(validated_output.error()));
    return nullptr;
  }

  auto* where = MakeGarbageCollected<MLOperator>(
      this, webnn::mojom::blink::Operation::Tag::kWhere);
  const auto output = MLOperand::ValidateAndCreateOutput(
      this, ComponentOperandTypeToBlink(validated_output->data_type),
      Vector<uint32_t>(validated_output->dimensions), where);
  if (!output.has_value()) {
    exception_state.ThrowTypeError(output.error());
    return nullptr;
  }
  where->Connect(std::move(inputs), {output.value()});
  return output.value();
}

ScriptPromise<MLGraph> MLGraphBuilder::build(
    ScriptState* script_state,
    const MLNamedOperands& named_outputs,
    ExceptionState& exception_state) {
  HeapVector<Member<const MLOperand>> outputs(named_outputs.size());
  base::ranges::transform(
      named_outputs, outputs.begin(),
      [](const auto& named_output) { return named_output.second; });
  THROW_AND_RETURN_TYPE_IF_ERROR(ValidateInputs(outputs),
                                 ScriptPromise<MLGraph>());

  ScopedMLTrace scoped_trace("MLGraphBuilder::build");
  if (!script_state->ContextIsValid()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Invalid script state");
    return ScriptPromise<MLGraph>();
  }

  auto* resolver = MakeGarbageCollected<ScriptPromiseResolver<MLGraph>>(
      script_state, exception_state.GetContext());
  auto promise = resolver->Promise();

  if (g_backend_for_testing) {
    g_backend_for_testing->BuildGraphImpl(ml_context_, named_outputs, resolver);
    return promise;
  }

#if BUILDFLAG(BUILD_WEBNN_WITH_XNNPACK)
  if (ml_context_->GetDeviceType() == V8MLDeviceType::Enum::kCpu) {
    MLGraphXnnpack::ValidateAndBuild(std::move(scoped_trace), ml_context_,
                                     named_outputs, resolver);
    return promise;
  }
#endif

  if (base::FeatureList::IsEnabled(
          webnn::mojom::features::kWebMachineLearningNeuralNetwork)) {
    MLGraphMojo::ValidateAndBuild(std::move(scoped_trace), ml_context_,
                                  named_outputs, resolver);
    return promise;
  }

  resolver->RejectWithDOMException(DOMExceptionCode::kNotSupportedError,
                                   "Not implemented");
  return promise;
}

// static
void MLGraphBuilder::SetBackendForTesting(
    MLGraphBuilder::BackendForTesting* backend_for_testing) {
  g_backend_for_testing = backend_for_testing;
}

// As specified in https://www.w3.org/TR/webnn/#mlgraphbuilder-validate-operand.
base::expected<void, String> MLGraphBuilder::ValidateInput(
    const MLOperand* input) {
  CHECK(input);
  if (input->Builder() != this) {
    return base::unexpected("Invalid input: Created from another builder.");
  }
  return base::ok();
}

base::expected<void, String> MLGraphBuilder::ValidateInputs(
    const HeapVector<Member<const MLOperand>>& inputs) {
  for (const MLOperand* input_to_validate : inputs) {
    RETURN_IF_ERROR(ValidateInput(input_to_validate));
  }
  return base::ok();
}

// As specified in
// https://www.w3.org/TR/webnn/#mlgraphbuilder-validate-activation.
base::expected<void, String> MLGraphBuilder::ValidateActivation(
    const MLActivation* activation) {
  CHECK(activation);
  if (activation->Operator()->Builder() != this) {
    return base::unexpected(
        "Invalid activation: Created from another builder.");
  }
  return base::ok();
}

base::expected<void, String> MLGraphBuilder::ValidateActivations(
    const HeapVector<Member<MLActivation>>& activations) {
  for (const MLActivation* activation_to_validate : activations) {
    RETURN_IF_ERROR(ValidateActivation(activation_to_validate));
  }
  return base::ok();
}

}  // namespace blink
