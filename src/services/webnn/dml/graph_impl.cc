// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/webnn/dml/graph_impl.h"

#include <winerror.h>
#include <algorithm>
#include <array>
#include <limits>

#include "base/bits.h"
#include "base/check.h"
#include "base/memory/ptr_util.h"
#include "base/notreached.h"
#include "base/numerics/safe_conversions.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"
#include "components/ml/webnn/graph_validation_utils.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "services/webnn/dml/command_queue.h"
#include "services/webnn/dml/command_recorder.h"
#include "services/webnn/dml/error.h"
#include "services/webnn/dml/graph_builder.h"
#include "services/webnn/dml/tensor_desc.h"
#include "services/webnn/dml/utils.h"
#include "services/webnn/error.h"
#include "services/webnn/public/mojom/webnn_error.mojom.h"
#include "services/webnn/webnn_utils.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/fp16/src/include/fp16.h"

namespace webnn::dml {
namespace {

using Microsoft::WRL::ComPtr;
using mojom::ComputeResult;
using mojom::CreateGraphResult;
using mojom::Operand;
using mojom::OperandPtr;
using mojom::Operation;

// A map of all mojom operands in `mojom::GraphInfo` using the mojom operand id
// as key.
using IdToOperandMap = base::flat_map<uint64_t, OperandPtr>;
// A map of all node outputs in `dml::GraphBuilder` using the mojom operand id
// as key.
using IdToNodeOutputMap = std::map<uint64_t, const NodeOutput*>;

constexpr const uint32_t kNhwcToNchwPermutation[] = {0, 3, 1, 2};
constexpr const uint32_t kNchwToNhwcPermutation[] = {0, 2, 3, 1};
// The `nhwc` input layout of regular conv2d is `ohwi` filter layout by default
// that need to be transposed to `oihw`.
constexpr const uint32_t kOhwiToOihwPermutation[] = {0, 3, 1, 2};
// The `nhwc` input layout of depthwise conv2d is `ihwo` filter layout by
// default that need to be transposed to `oihw`.
constexpr const uint32_t kIhwoToOihwPermutation[] = {3, 0, 1, 2};

DML_TENSOR_DATA_TYPE GetTensorDataType(Operand::DataType type) {
  switch (type) {
    case Operand::DataType::kFloat32:
      return DML_TENSOR_DATA_TYPE_FLOAT32;
    case Operand::DataType::kFloat16:
      return DML_TENSOR_DATA_TYPE_FLOAT16;
    case Operand::DataType::kInt8:
      return DML_TENSOR_DATA_TYPE_INT8;
    case Operand::DataType::kUint8:
      return DML_TENSOR_DATA_TYPE_UINT8;
    case Operand::DataType::kInt64:
      return DML_TENSOR_DATA_TYPE_INT64;
    case Operand::DataType::kUint64:
      return DML_TENSOR_DATA_TYPE_UINT64;
    case Operand::DataType::kInt32:
      return DML_TENSOR_DATA_TYPE_INT32;
    case Operand::DataType::kUint32:
      return DML_TENSOR_DATA_TYPE_UINT32;
    default:
      DLOG(ERROR) << "This data type is not supported.";
      NOTREACHED_NORETURN();
  }
}

DML_REDUCE_FUNCTION MapReduceKindToReduceFuntion(mojom::Reduce::Kind kind) {
  switch (kind) {
    case mojom::Reduce::Kind::kL1:
      return DML_REDUCE_FUNCTION_L1;
    case mojom::Reduce::Kind::kL2:
      return DML_REDUCE_FUNCTION_L2;
    case mojom::Reduce::Kind::kLogSum:
      return DML_REDUCE_FUNCTION_LOG_SUM;
    case mojom::Reduce::Kind::kLogSumExp:
      return DML_REDUCE_FUNCTION_LOG_SUM_EXP;
    case mojom::Reduce::Kind::kMax:
      return DML_REDUCE_FUNCTION_MAX;
    case mojom::Reduce::Kind::kMean:
      return DML_REDUCE_FUNCTION_AVERAGE;
    case mojom::Reduce::Kind::kMin:
      return DML_REDUCE_FUNCTION_MIN;
    case mojom::Reduce::Kind::kProduct:
      return DML_REDUCE_FUNCTION_MULTIPLY;
    case mojom::Reduce::Kind::kSum:
      return DML_REDUCE_FUNCTION_SUM;
    case mojom::Reduce::Kind::kSumSquare:
      return DML_REDUCE_FUNCTION_SUM_SQUARE;
  }
  NOTREACHED_NORETURN();
}

// Calculate the total byte length of buffers and the D3D12_RANGE for each
// buffer, all with the required alignment.
template <typename Map>
std::optional<AlignedByteLength<typename Map::key_type>>
CalculateAlignedByteLength(const Map& buffer_to_byte_length_map) {
  base::CheckedNumeric<size_t> total_byte_length(0);
  std::map<typename Map::key_type, D3D12_RANGE> key_to_d3d12_range_map;

  for (auto& [buffer, byte_length] : buffer_to_byte_length_map) {
    auto& d3d12_range = key_to_d3d12_range_map[buffer];
    d3d12_range.Begin = total_byte_length.ValueOrDie();

    // The buffer has a minimum base address alignment requirement of 16 bytes
    // in the macro `DML_MINIMUM_BUFFER_TENSOR_ALIGNMENT`:
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-directml-constants
    total_byte_length += base::bits::AlignUp<size_t>(
        byte_length, DML_MINIMUM_BUFFER_TENSOR_ALIGNMENT);
    if (!total_byte_length.IsValid()) {
      DLOG(ERROR) << "Failed to calculate the total byte length.";
      return std::nullopt;
    }

    // The aligned byte length calculated with `End` sub `Begin` attribute is
    // used to set the `SizeInBytes` field of `DML_BUFFER_BINDING`.
    d3d12_range.End = total_byte_length.ValueOrDie();
  }

  return AlignedByteLength<typename Map::key_type>{
      .total_byte_length = total_byte_length.ValueOrDie(),
      .key_to_d3d12_range_map = std::move(key_to_d3d12_range_map)};
}

struct UploadAndDefaultBuffers {
  ComPtr<ID3D12Resource> upload_buffer;
  ComPtr<ID3D12Resource> default_buffer;
};

// Upload constants buffers in one Direct3D 12 committed resource, the
// DML_BUFFER_BINDING specifies a resource binding described by a range of bytes
// in the single buffer. For GPU supports UMA, pass a custom upload buffer via
// `buffer_variant` for both constants uploading and binding. For GPU doesn't
// support UMA, pass a upload buffer and a default buffer via `buffer_variant`
// for uploading and binding separately.
std::optional<std::map<uint64_t, DML_BUFFER_BINDING>>
UploadAndCreateConstantBufferBinding(
    CommandRecorder* command_recorder,
    const base::flat_map<uint64_t, mojo_base::BigBuffer>& key_to_buffer_map,
    const AlignedByteLength<uint64_t>& aligned_byte_length,
    absl::variant<UploadAndDefaultBuffers, ComPtr<ID3D12Resource>>
        buffer_variant) {
  // Map entire resource to copy the array buffer of constant/input one by one
  // with byte offset.
  void* mapped_buffer = nullptr;
  ID3D12Resource* buffer_to_map = nullptr;
  ID3D12Resource* buffer_to_bind = nullptr;
  ComPtr<ID3D12Resource> cpu_buffer;
  ComPtr<ID3D12Resource> upload_buffer;
  ComPtr<ID3D12Resource> default_buffer;
  if (absl::holds_alternative<ComPtr<ID3D12Resource>>(buffer_variant)) {
    cpu_buffer = std::move(absl::get<ComPtr<ID3D12Resource>>(buffer_variant));
    buffer_to_map = cpu_buffer.Get();
    buffer_to_bind = buffer_to_map;
  } else {
    upload_buffer = std::move(
        absl::get<UploadAndDefaultBuffers>(buffer_variant).upload_buffer);
    default_buffer = std::move(
        absl::get<UploadAndDefaultBuffers>(buffer_variant).default_buffer);
    buffer_to_map = upload_buffer.Get();
    buffer_to_bind = default_buffer.Get();
  }
  CHECK(buffer_to_map);
  CHECK(buffer_to_bind);

  HRESULT hr = buffer_to_map->Map(0, nullptr, &mapped_buffer);
  if (FAILED(hr)) {
    DLOG(ERROR) << "Failed to map buffer for inputs: "
                << logging::SystemErrorCodeToString(hr);
    return std::nullopt;
  }

  std::map<uint64_t, DML_BUFFER_BINDING> key_to_buffer_binding_map;
  for (auto& [key, buffer] : key_to_buffer_map) {
    // Copy the input data to the upload heap with byte offset
    const auto& d3d12_range =
        aligned_byte_length.key_to_d3d12_range_map.at(key);
    memcpy(static_cast<uint8_t*>(mapped_buffer) + d3d12_range.Begin,
           buffer.data(), buffer.size());
    // Create the buffer binding for each constant/input and push back into the
    // DML_BUFFER_BINDING array.
    auto size_in_bytes = d3d12_range.End - d3d12_range.Begin;
    key_to_buffer_binding_map[key] =
        DML_BUFFER_BINDING{.Buffer = buffer_to_bind,
                           .Offset = d3d12_range.Begin,
                           .SizeInBytes = size_in_bytes};
  }
  buffer_to_map->Unmap(0, nullptr);

  if (absl::holds_alternative<ComPtr<ID3D12Resource>>(buffer_variant)) {
    CHECK(cpu_buffer);
    command_recorder->GetCommandQueue()->ReferenceUntilCompleted(
        std::move(cpu_buffer));
  } else {
    CHECK(default_buffer);
    CHECK(upload_buffer);
    UploadBufferWithBarrier(command_recorder, std::move(default_buffer),
                            std::move(upload_buffer),
                            aligned_byte_length.total_byte_length);
  }

  return key_to_buffer_binding_map;
}

HRESULT MapAndCopyInputDataToBuffer(
    const base::flat_map<std::string, mojo_base::BigBuffer>& named_inputs,
    const std::map<std::string, D3D12_RANGE>& input_name_to_d3d12_range_map,
    ID3D12Resource* buffer) {
  // Map entire resource to copy the array buffer of input one by one
  // with byte offset.
  void* mapped_buffer = nullptr;
  CHECK(buffer);
  RETURN_IF_FAILED(buffer->Map(0, nullptr, &mapped_buffer));

  for (auto& [name, input] : named_inputs) {
    // Copy the input data to the upload heap with byte offset
    const auto& d3d12_range = input_name_to_d3d12_range_map.at(name);
    memcpy(static_cast<uint8_t*>(mapped_buffer) + d3d12_range.Begin,
           input.data(), input.size());
  }
  buffer->Unmap(0, nullptr);

  return S_OK;
}

// Define some methods like CreateInputNode and CreateOperatorNodeForRelu here
// to focus on converting the mojo graph struct to corresponding DML graph node
// by using dml::GraphBuilder as a helper. dml::GraphBuilder should be decoupled
// from mojo graph structs and focus on manipulating DML graph structs.
//
// The return value is the GraphInputIndex assigned by graph builder.
uint32_t CreateInputNode(const IdToOperandMap& id_to_operand_map,
                         uint64_t input_id,
                         GraphBuilder& graph_builder,
                         IdToNodeOutputMap& id_to_node_output_map) {
  const OperandPtr& operand = id_to_operand_map.at(input_id);
  // If the operand is constant, the tensor is identified by
  // DML_TENSOR_FLAG_OWNED_BY_DML which must be bound to the binding table
  // during the graph initialization, and not during execution.
  DML_TENSOR_FLAGS flags = operand->kind == Operand::Kind::kConstant
                               ? DML_TENSOR_FLAG_OWNED_BY_DML
                               : DML_TENSOR_FLAG_NONE;
  TensorDesc input_tensor_desc(GetTensorDataType(operand->data_type), flags,
                               operand->dimensions);
  const InputNode* input_node = graph_builder.CreateInputNode();
  CHECK(input_node);
  const NodeOutput* node_output =
      graph_builder.CreateNodeOutput(input_node, std::move(input_tensor_desc));
  CHECK(node_output);
  id_to_node_output_map[input_id] = std::move(node_output);
  return input_node->GetGraphInputIndex();
}

const NodeOutput* GetNodeOutputForOperand(
    const IdToNodeOutputMap& id_to_node_output_map,
    uint64_t operand_id) {
  const auto input_iterator = id_to_node_output_map.find(operand_id);
  CHECK(input_iterator != id_to_node_output_map.end());
  CHECK(input_iterator->second);
  return input_iterator->second;
}

// Build a one-element constant operand with specified rank for float value and
// add it into the graph info. For example, if the rank is 3, the operand
// dimensions would be {1, 1, 1}.
uint64_t BuildConstantOperandForFloatValue(mojom::GraphInfoPtr& graph_info,
                                           uint64_t& next_operand_id,
                                           Operand::DataType data_type,
                                           size_t rank,
                                           float value) {
  OperandPtr constant_operand = Operand::New();
  constant_operand->kind = Operand::Kind::kConstant;
  constant_operand->dimensions = std::vector<uint32_t>(rank, 1);
  constant_operand->data_type = data_type;

  uint64_t constant_id = next_operand_id++;
  CHECK(graph_info->id_to_operand_map
            .try_emplace(constant_id, std::move(constant_operand))
            .second);

  mojo_base::BigBuffer buffer;

  switch (data_type) {
    case Operand::DataType::kFloat32: {
      buffer = mojo_base::BigBuffer(base::make_span(
          reinterpret_cast<const uint8_t*>(&value), sizeof(value)));
      break;
    }
    case Operand::DataType::kFloat16: {
      uint16_t fp16_value = fp16_ieee_from_fp32_value(value);
      buffer = mojo_base::BigBuffer(base::make_span(
          reinterpret_cast<const uint8_t*>(&fp16_value), sizeof(fp16_value)));
      break;
    }
    default:
      DLOG(ERROR)
          << "The data type must be one of the floating point data types.";
      NOTREACHED_NORETURN();
  }

  CHECK(graph_info->constant_id_to_buffer_map
            .try_emplace(constant_id, std::move(buffer))
            .second);

  return constant_id;
}

const TensorDesc CreateOutputTensorDesc(const IdToOperandMap& id_to_operand_map,
                                        uint64_t output_id) {
  const OperandPtr& output_operand = id_to_operand_map.at(output_id);
  return TensorDesc(GetTensorDataType(output_operand->data_type),
                    output_operand->dimensions);
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForArgMinMax(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ArgMinMaxPtr& arg_min_max,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, arg_min_max->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  const uint64_t output_id = arg_min_max->output_operand_id;
  const auto& output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  const auto axes = arg_min_max->axes;
  // Determine output sizes. Ignore output_desc->dimensions for the dimensions,
  // since DirectML expects the output dimensions to have the same rank as the
  // input, and output_desc->dimensions may have removed dimensions if
  // keepDimensions was false.
  std::vector<uint32_t> output_dimensions = input_tensor_desc.GetDimensions();
  for (uint32_t axis : axes) {
    CHECK_LT(axis, output_dimensions.size());
    output_dimensions[axis] = 1u;
  }

  TensorDesc new_output_tensor_desc(output_tensor_desc.GetDataType(),
                                    std::move(output_dimensions));

  std::array<const NodeOutput*, 1> inputs = {input};
  DML_ARGMAX_OPERATOR_DESC operator_desc = {};
  operator_desc.InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
  operator_desc.OutputTensor = &new_output_tensor_desc.GetDMLTensorDesc(),
  operator_desc.AxisCount = axes.size();
  operator_desc.Axes = axes.data();
  operator_desc.AxisDirection =
      arg_min_max->select_last_index
          ? DML_AXIS_DIRECTION::DML_AXIS_DIRECTION_DECREASING
          : DML_AXIS_DIRECTION::DML_AXIS_DIRECTION_INCREASING;
  DML_OPERATOR_TYPE operator_type;
  switch (arg_min_max->kind) {
    case mojom::ArgMinMax_Kind::kMin: {
      operator_type = DML_OPERATOR_ARGMIN;
      break;
    }
    case mojom::ArgMinMax_Kind::kMax: {
      operator_type = DML_OPERATOR_ARGMAX;
      break;
    }
  }
  const OperatorNode* arg_min_max_node =
      graph_builder.CreateOperatorNode(operator_type, &operator_desc, inputs);
  if (!arg_min_max_node) {
    return base::unexpected(mojom::Error::New(
        mojom::Error::Code::kUnknownError,
        "Failed to create " + OpKindToString(arg_min_max->kind) +
            " operator."));
  }

  const NodeOutput* output =
      graph_builder.CreateNodeOutput(arg_min_max_node, output_tensor_desc);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

struct ActivationOperatorDesc {
  absl::variant<DML_ACTIVATION_ELU_OPERATOR_DESC,
                DML_ACTIVATION_HARD_SIGMOID_OPERATOR_DESC,
                DML_ACTIVATION_LEAKY_RELU_OPERATOR_DESC,
                DML_ACTIVATION_LINEAR_OPERATOR_DESC,
                DML_ACTIVATION_RELU_OPERATOR_DESC,
                DML_ACTIVATION_SIGMOID_OPERATOR_DESC,
                DML_ACTIVATION_SOFTPLUS_OPERATOR_DESC,
                DML_ACTIVATION_SOFTSIGN_OPERATOR_DESC,
                DML_ACTIVATION_TANH_OPERATOR_DESC>
      desc;

  DML_OPERATOR_DESC GetActivationDmlDesc() const {
    if (absl::holds_alternative<DML_ACTIVATION_ELU_OPERATOR_DESC>(desc)) {
      return {DML_OPERATOR_ACTIVATION_ELU,
              &absl::get<DML_ACTIVATION_ELU_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<
                   DML_ACTIVATION_HARD_SIGMOID_OPERATOR_DESC>(desc)) {
      return {DML_OPERATOR_ACTIVATION_HARD_SIGMOID,
              &absl::get<DML_ACTIVATION_HARD_SIGMOID_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_LEAKY_RELU_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_LEAKY_RELU,
              &absl::get<DML_ACTIVATION_LEAKY_RELU_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_LINEAR_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_LINEAR,
              &absl::get<DML_ACTIVATION_LINEAR_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_RELU_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_RELU,
              &absl::get<DML_ACTIVATION_RELU_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_SIGMOID_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_SIGMOID,
              &absl::get<DML_ACTIVATION_SIGMOID_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_SOFTPLUS_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_SOFTPLUS,
              &absl::get<DML_ACTIVATION_SOFTPLUS_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_SOFTSIGN_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_SOFTSIGN,
              &absl::get<DML_ACTIVATION_SOFTSIGN_OPERATOR_DESC>(desc)};
    } else if (absl::holds_alternative<DML_ACTIVATION_TANH_OPERATOR_DESC>(
                   desc)) {
      return {DML_OPERATOR_ACTIVATION_TANH,
              &absl::get<DML_ACTIVATION_TANH_OPERATOR_DESC>(desc)};
    } else {
      NOTREACHED_NORETURN() << "The activation type is not supported.";
    }
  }
};

// DML_OPERATOR_ELEMENT_WISE_CLIP will be supported after the DirectML version
// upper than DML_FEATURE_LEVEL_6_0.
// https://learn.microsoft.com/en-us/windows/ai/directml/dml-feature-level-history#dml_feature_level_6_0
base::expected<ActivationOperatorDesc, mojom::ErrorPtr>
CreateActivationOperatorDesc(const mojom::ActivationPtr& activation) {
  CHECK(activation);
  switch (activation->which()) {
    case mojom::Activation::Tag::kElu:
      return ActivationOperatorDesc{.desc = DML_ACTIVATION_ELU_OPERATOR_DESC{
                                        .Alpha = activation->get_elu()->alpha}};
    case mojom::Activation::Tag::kHardSigmoid:
      return ActivationOperatorDesc{
          .desc = DML_ACTIVATION_HARD_SIGMOID_OPERATOR_DESC{
              .Alpha = activation->get_hard_sigmoid()->alpha,
              .Beta = activation->get_hard_sigmoid()->beta}};
    case mojom::Activation::Tag::kLeakyRelu:
      return ActivationOperatorDesc{
          .desc = DML_ACTIVATION_LEAKY_RELU_OPERATOR_DESC{
              .Alpha = activation->get_leaky_relu()->alpha}};
    case mojom::Activation::Tag::kLinear:
      return ActivationOperatorDesc{
          .desc = DML_ACTIVATION_LINEAR_OPERATOR_DESC{
              .Alpha = activation->get_linear()->alpha,
              .Beta = activation->get_linear()->beta}};
    case mojom::Activation::Tag::kRelu:
      return ActivationOperatorDesc{.desc =
                                        DML_ACTIVATION_RELU_OPERATOR_DESC{}};
    case mojom::Activation::Tag::kSigmoid:
      return ActivationOperatorDesc{.desc =
                                        DML_ACTIVATION_SIGMOID_OPERATOR_DESC{}};
    case mojom::Activation::Tag::kSoftplus:
      return ActivationOperatorDesc{
          .desc = DML_ACTIVATION_SOFTPLUS_OPERATOR_DESC{
              .Steepness = activation->get_softplus()->steepness}};
    case mojom::Activation::Tag::kSoftsign:
      return ActivationOperatorDesc{
          .desc = DML_ACTIVATION_SOFTSIGN_OPERATOR_DESC{}};
    case mojom::Activation::Tag::kTanh:
      return ActivationOperatorDesc{.desc =
                                        DML_ACTIVATION_TANH_OPERATOR_DESC{}};
    default:
      return base::unexpected(
          CreateError(mojom::Error::Code::kNotSupportedError,
                      "The fused activation type is not supported."));
  }
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForBatchNormalization(
    const mojom::BatchNormalizationPtr& batch_normalization,
    mojom::GraphInfoPtr& graph_info,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map,
    std::unordered_map<uint64_t, uint32_t>& constant_id_to_input_index_map,
    uint64_t& next_operand_id) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, batch_normalization->input_operand_id);
  const TensorDesc& input_tensor_desc = input->GetTensorDesc();
  const auto input_rank = input_tensor_desc.GetDimensions().size();

  auto& id_to_operand_map = graph_info->id_to_operand_map;
  uint64_t output_id = batch_normalization->output_operand_id;
  const OperandPtr& output_operand = id_to_operand_map.at(output_id);
  Operand::DataType data_type = output_operand->data_type;
  const TensorDesc output_tensor_desc(GetTensorDataType(data_type),
                                      output_operand->dimensions);

  const NodeOutput* mean = GetNodeOutputForOperand(
      id_to_node_output_map, batch_normalization->mean_operand_id);
  auto mean_tensor_desc = mean->GetTensorDesc();
  auto mean_rank = mean_tensor_desc.GetDimensions().size();
  CHECK_EQ(mean_rank, 1U);

  auto axis = batch_normalization->axis;
  uint32_t axes[1] = {axis};

  // In WebNN spec, mean operand is specified as a 1-D tensor and its size equal
  // to the size of the input dimension denoted by axis. But for DML,
  // InputTensor and MeanTensor must have the same DimensionCount -
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_batch_normalization_operator_desc.
  mean_tensor_desc.MakeBroadcastCompatible(input_rank, axes);

  const NodeOutput* variance = GetNodeOutputForOperand(
      id_to_node_output_map, batch_normalization->variance_operand_id);
  auto variance_tensor_desc = variance->GetTensorDesc();
  auto variance_rank = variance_tensor_desc.GetDimensions().size();
  CHECK_EQ(variance_rank, 1U);

  // In WebNN spec, variance operand is specified as a 1-D tensor and its size
  // equal to the size of the input dimension denoted by axis. But for DML,
  // InputTensor and VarianceTensor must have the same DimensionCount -
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_batch_normalization_operator_desc.
  variance_tensor_desc.MakeBroadcastCompatible(input_rank, axes);

  uint64_t scale_operand_id;
  if (batch_normalization->scale_operand_id.has_value()) {
    scale_operand_id = batch_normalization->scale_operand_id.value();
  } else {
    // If the scale is not present, create a constant operand for scale and
    // insert the operand into the graph.
    scale_operand_id = BuildConstantOperandForFloatValue(
        graph_info, next_operand_id, data_type,
        /*rank*/ 1, /*default scale*/ 1.0);

    // Create an input node for the scale operand and store the assigned input
    // index in `constant_id_to_input_index_map`, which will be used for
    // constant buffer binding.
    uint32_t scale_input_index =
        CreateInputNode(id_to_operand_map, scale_operand_id, graph_builder,
                        id_to_node_output_map);
    CHECK(constant_id_to_input_index_map
              .try_emplace(scale_operand_id, scale_input_index)
              .second);
  }

  const NodeOutput* scale =
      GetNodeOutputForOperand(id_to_node_output_map, scale_operand_id);
  auto scale_tensor_desc = scale->GetTensorDesc();
  auto scale_rank = scale_tensor_desc.GetDimensions().size();
  CHECK_EQ(scale_rank, 1U);

  // In WebNN spec, scale operand is specified as a 1-D tensor and its size
  // equal to the size of the input dimension denoted by axis. But for DML,
  // InputTensor and ScaleTensor must have the same DimensionCount -
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_batch_normalization_operator_desc.
  scale_tensor_desc.MakeBroadcastCompatible(input_rank, axes);

  uint64_t bias_operand_id;
  if (batch_normalization->bias_operand_id.has_value()) {
    bias_operand_id = batch_normalization->bias_operand_id.value();
  } else {
    // If the bias is not present, create a constant operand for bias and insert
    // the operand into the graph.
    bias_operand_id = BuildConstantOperandForFloatValue(
        graph_info, next_operand_id, data_type,
        /*rank*/ 1, /*default bias*/ 0);

    // Create an input node for the bias operand and store the assigned input
    // index in `constant_id_to_input_index_map`, which will be used for
    // constant buffer binding.
    uint32_t bias_input_index =
        CreateInputNode(id_to_operand_map, bias_operand_id, graph_builder,
                        id_to_node_output_map);
    CHECK(constant_id_to_input_index_map
              .try_emplace(bias_operand_id, bias_input_index)
              .second);
  }

  const NodeOutput* bias =
      GetNodeOutputForOperand(id_to_node_output_map, bias_operand_id);
  auto bias_tensor_desc = bias->GetTensorDesc();
  auto bias_rank = bias_tensor_desc.GetDimensions().size();
  CHECK_EQ(bias_rank, 1U);

  // In WebNN spec, bias operand is specified as a 1-D tensor and its size
  // equal to the size of the input dimension denoted by axis. But for DML,
  // InputTensor and BiasTensor must have the same DimensionCount -
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_batch_normalization_operator_desc.
  bias_tensor_desc.MakeBroadcastCompatible(input_rank, axes);

  std::array<const NodeOutput*, 5> inputs = {input, mean, variance, scale,
                                             bias};

  std::optional<ActivationOperatorDesc> activation_operator_desc;
  std::optional<DML_OPERATOR_DESC> activation_dml_desc;
  if (batch_normalization->activation) {
    auto create_activation_result =
        CreateActivationOperatorDesc(batch_normalization->activation);
    if (!create_activation_result.has_value()) {
      return base::unexpected(std::move(create_activation_result.error()));
    }

    activation_operator_desc = std::move(create_activation_result.value());
    activation_dml_desc = activation_operator_desc->GetActivationDmlDesc();
  }

  DML_BATCH_NORMALIZATION_OPERATOR_DESC batch_normalization_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .MeanTensor = &mean_tensor_desc.GetDMLTensorDesc(),
      .VarianceTensor = &variance_tensor_desc.GetDMLTensorDesc(),
      .ScaleTensor = &scale_tensor_desc.GetDMLTensorDesc(),
      .BiasTensor = &bias_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      // Spatial is used to specify whether locations are spatial.
      // This parameter was deprecated in DML_FEATURE_LEVEL_4_0, and has no
      // effect.
      .Spatial = true,
      .Epsilon = batch_normalization->epsilon,
      .FusedActivation =
          activation_dml_desc ? &activation_dml_desc.value() : nullptr,
  };

  const OperatorNode* batch_normalization_node =
      graph_builder.CreateOperatorNode(DML_OPERATOR_BATCH_NORMALIZATION,
                                       &batch_normalization_operator_desc,
                                       inputs);
  if (!batch_normalization_node) {
    return base::unexpected(
        mojom::Error::New(mojom::Error::Code::kUnknownError,
                          "Failed to create batch normalization operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      batch_normalization_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForClamp(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ClampPtr& clamp,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, clamp->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = clamp->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_ELEMENT_WISE_CLIP_OPERATOR_DESC clamp_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      // No scale or bias applies to the input.
      .ScaleBias = nullptr,
      .Min = clamp->min_value,
      .Max = clamp->max_value};
  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* clamp_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ELEMENT_WISE_CLIP, &clamp_operator_desc, inputs);
  if (!clamp_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create clamp operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      clamp_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForConcat(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ConcatPtr& concat,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const auto& input_operand_ids = concat->input_operand_ids;
  size_t input_num = input_operand_ids.size();

  std::vector<const NodeOutput*> inputs;
  std::vector<DML_TENSOR_DESC> input_dml_tensor_descs;
  inputs.reserve(input_num);
  input_dml_tensor_descs.reserve(input_num);

  for (const auto& input_operand_id : input_operand_ids) {
    const NodeOutput* input =
        GetNodeOutputForOperand(id_to_node_output_map, input_operand_id);
    inputs.push_back(input);
    input_dml_tensor_descs.push_back(input->GetTensorDesc().GetDMLTensorDesc());
  }

  uint64_t output_id = concat->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_JOIN_OPERATOR_DESC concat_operator_desc{
      .InputCount = base::checked_cast<uint32_t>(input_dml_tensor_descs.size()),
      .InputTensors = input_dml_tensor_descs.data(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Axis = concat->axis};

  const OperatorNode* concat_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_JOIN, &concat_operator_desc, inputs);
  if (!concat_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create concat operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      concat_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForConv2d(
    const IdToOperandMap& id_to_operand_map,
    const mojom::Conv2dPtr& conv2d,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, conv2d->input_operand_id);
  // The input tensor description may be transposed.
  auto input_tensor_desc = input->GetTensorDesc();
  CHECK_EQ(input_tensor_desc.GetDimensions().size(), 4u);

  const NodeOutput* filter =
      GetNodeOutputForOperand(id_to_node_output_map, conv2d->filter_operand_id);
  auto filter_tensor_desc = filter->GetTensorDesc();

  uint64_t output_id = conv2d->output_operand_id;
  // The output tensor description may be transposed.
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  CHECK_EQ(output_tensor_desc.GetDimensions().size(), 4u);

  std::vector<const NodeOutput*> inputs = {input, filter};
  std::optional<TensorDesc> reshaped_bias_tensor_desc;
  auto& bias_operand_id = conv2d->bias_operand_id;
  if (bias_operand_id) {
    const auto bias_node_output_iterator =
        id_to_node_output_map.find(bias_operand_id.value());
    CHECK(bias_node_output_iterator != id_to_node_output_map.end());
    const NodeOutput* bias_node_output = bias_node_output_iterator->second;
    CHECK(bias_node_output);
    const auto& bias_tensor_desc = bias_node_output->GetTensorDesc();
    const auto& bias_dims = bias_tensor_desc.GetDimensions();
    CHECK_EQ(bias_dims.size(), 1u);

    // In WebNN spec bias specifies the additional 1-D tensor with the shape of
    // {outputChannels}. But for DML the expected dimensions of the BiasTensor
    // are { 1, OutputChannelCount, 1, 1 } for 4D. So reshape the bias:
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_convolution_operator_desc
    std::vector<uint32_t> reshaped_bias_dims = {1, bias_dims[0], 1, 1};
    reshaped_bias_tensor_desc =
        TensorDesc(bias_tensor_desc.GetDataType(), bias_tensor_desc.GetFlags(),
                   std::move(reshaped_bias_dims));

    const NodeOutput* reshaped_bias_node_output =
        graph_builder.CreateNodeOutput(&bias_node_output->GetNode(),
                                       reshaped_bias_tensor_desc.value());
    inputs.push_back(reshaped_bias_node_output);
  }

  switch (conv2d->input_layout) {
    case mojom::InputOperandLayout::kChannelsFirst: {
      break;
    }
    // DML convolution operator only support nchw layout according to
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_convolution_operator_desc
    //
    // To support other layouts, we can transpose the input and output
    // tensors
    case mojom::InputOperandLayout::kChannelsLast: {
      if (conv2d->kind == mojom::Conv2d::Kind::kDirect) {
        const uint32_t input_channels = input_tensor_desc.GetDimensions()[3];
        const uint32_t output_channels = output_tensor_desc.GetDimensions()[3];
        const bool depthwise = webnn::IsDepthwiseConv2d(
            input_channels, output_channels, conv2d->groups);
        if (depthwise) {
          // The filter layout is `ihwo` for depthwise conv2d.
          filter_tensor_desc.Transpose(kIhwoToOihwPermutation);
        } else {
          // The filter layout is `ohwi` for regular conv2d.
          filter_tensor_desc.Transpose(kOhwiToOihwPermutation);
        }
      }
      input_tensor_desc.Transpose(kNhwcToNchwPermutation);
      output_tensor_desc.Transpose(kNhwcToNchwPermutation);
      break;
    }
  }

  std::array<uint32_t, 2> strides = {conv2d->strides->height,
                                     conv2d->strides->width};
  std::array<uint32_t, 2> dilations = {conv2d->dilations->height,
                                       conv2d->dilations->width};
  std::array<uint32_t, 2> start_padding = {conv2d->padding->beginning->height,
                                           conv2d->padding->beginning->width};
  std::array<uint32_t, 2> end_padding = {conv2d->padding->ending->height,
                                         conv2d->padding->ending->width};

  // The outputSizes of WebNN convTranspose2d specifies the sizes of the last
  // two dimensions of the output tensor but the outputPadding of DirectML
  // convolution applies a zero padding to the result of the operator. Since
  // graph builder will explicitly pass in the output tensor shape anyway. So,
  // there is no ambiguity of the output shape and we set the output_padding to
  // {0, 0}:
  // https://www.w3.org/TR/webnn/#dom-mlconvtranspose2doptions-outputpadding
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_convolution_operator_desc
  std::array<uint32_t, 2> default_out_padding = {0, 0};

  std::optional<ActivationOperatorDesc> activation_operator_desc;
  std::optional<DML_OPERATOR_DESC> activation_dml_desc;
  if (conv2d->activation) {
    auto create_activation_result =
        CreateActivationOperatorDesc(conv2d->activation);
    if (!create_activation_result.has_value()) {
      return base::unexpected(std::move(create_activation_result.error()));
    }

    activation_operator_desc = std::move(create_activation_result.value());
    activation_dml_desc = activation_operator_desc->GetActivationDmlDesc();
  }

  DML_CONVOLUTION_DIRECTION conv2d_direction;
  switch (conv2d->kind) {
    case mojom::Conv2d::Kind::kDirect:
      conv2d_direction =
          DML_CONVOLUTION_DIRECTION::DML_CONVOLUTION_DIRECTION_FORWARD;
      break;
    case mojom::Conv2d::Kind::kTransposed:
      conv2d_direction =
          DML_CONVOLUTION_DIRECTION::DML_CONVOLUTION_DIRECTION_BACKWARD;
      break;
  }

  DML_CONVOLUTION_OPERATOR_DESC conv2d_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .FilterTensor = &filter_tensor_desc.GetDMLTensorDesc(),
      .BiasTensor = (reshaped_bias_tensor_desc.has_value())
                        ? &reshaped_bias_tensor_desc->GetDMLTensorDesc()
                        : nullptr,
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Mode = DML_CONVOLUTION_MODE_CROSS_CORRELATION,
      .Direction = conv2d_direction,
      .DimensionCount =
          2u, /*Determines the size of the Strides, Dilations, StartPadding,
                 EndPadding, and OutputPadding arrays.*/
      .Strides = strides.data(),
      .Dilations = dilations.data(),
      .StartPadding = start_padding.data(),
      .EndPadding = end_padding.data(),
      .OutputPadding = default_out_padding.data(),
      .GroupCount = conv2d->groups,
      .FusedActivation =
          activation_dml_desc ? &activation_dml_desc.value() : nullptr};

  const OperatorNode* conv2d_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_CONVOLUTION, &conv2d_operator_desc, inputs);
  if (!conv2d_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create conv2d operator."));
  }

  if (conv2d->input_layout == mojom::InputOperandLayout::kChannelsLast) {
    // Transpose the output tensor from nchw to nhwc layout.
    output_tensor_desc.Transpose(kNchwToNhwcPermutation);
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      conv2d_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

template <typename DML_OPERATOR_DESC>
const OperatorNode* CreateBinaryOperator(const TensorDesc& a_tensor,
                                         const TensorDesc& b_tensor,
                                         const TensorDesc& output_tensor,
                                         GraphBuilder& graph_builder,
                                         DML_OPERATOR_TYPE operator_type,
                                         base::span<const NodeOutput*> inputs) {
  DML_OPERATOR_DESC binary_operator_desc{
      .ATensor = &a_tensor.GetDMLTensorDesc(),
      .BTensor = &b_tensor.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor.GetDMLTensorDesc()};
  return graph_builder.CreateOperatorNode(operator_type, &binary_operator_desc,
                                          inputs);
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForBinary(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ElementWiseBinaryPtr& operation,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  // The input a and b tensor descriptions may be broadcasted.
  const NodeOutput* input_a =
      GetNodeOutputForOperand(id_to_node_output_map, operation->lhs_operand);
  auto input_a_tensor_desc = input_a->GetTensorDesc();
  const NodeOutput* input_b =
      GetNodeOutputForOperand(id_to_node_output_map, operation->rhs_operand);
  auto input_b_tensor_desc = input_b->GetTensorDesc();

  uint64_t output_id = operation->output_operand;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  auto output_dimensions = output_tensor_desc.GetDimensions();
  if (input_a_tensor_desc.GetDimensions() != output_dimensions) {
    input_a_tensor_desc.BroadcastTo(output_dimensions);
  }
  if (input_b_tensor_desc.GetDimensions() != output_dimensions) {
    input_b_tensor_desc.BroadcastTo(output_dimensions);
  }

  const OperatorNode* binary_node = nullptr;
  std::array<const NodeOutput*, 2> inputs = {input_a, input_b};
  switch (operation->kind) {
    case mojom::ElementWiseBinary::Kind::kAdd: {
      binary_node = CreateBinaryOperator<DML_ELEMENT_WISE_ADD_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_ADD, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kDiv: {
      binary_node = CreateBinaryOperator<DML_ELEMENT_WISE_DIVIDE_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_DIVIDE, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kMax: {
      binary_node = CreateBinaryOperator<DML_ELEMENT_WISE_MAX_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_MAX, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kMin: {
      binary_node = CreateBinaryOperator<DML_ELEMENT_WISE_MIN_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_MIN, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kMul: {
      binary_node =
          CreateBinaryOperator<DML_ELEMENT_WISE_MULTIPLY_OPERATOR_DESC>(
              input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
              graph_builder, DML_OPERATOR_ELEMENT_WISE_MULTIPLY, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kSub: {
      binary_node =
          CreateBinaryOperator<DML_ELEMENT_WISE_SUBTRACT_OPERATOR_DESC>(
              input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
              graph_builder, DML_OPERATOR_ELEMENT_WISE_SUBTRACT, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kPow: {
      DML_ELEMENT_WISE_POW_OPERATOR_DESC element_wise_operator_desc{
          .InputTensor = &input_a_tensor_desc.GetDMLTensorDesc(),
          .ExponentTensor = &input_b_tensor_desc.GetDMLTensorDesc(),
          .OutputTensor = &output_tensor_desc.GetDMLTensorDesc()};
      binary_node = graph_builder.CreateOperatorNode(
          DML_OPERATOR_ELEMENT_WISE_POW, &element_wise_operator_desc, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kEqual: {
      binary_node =
          CreateBinaryOperator<DML_ELEMENT_WISE_LOGICAL_EQUALS_OPERATOR_DESC>(
              input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
              graph_builder, DML_OPERATOR_ELEMENT_WISE_LOGICAL_EQUALS, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kGreater: {
      binary_node = CreateBinaryOperator<
          DML_ELEMENT_WISE_LOGICAL_GREATER_THAN_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_LOGICAL_GREATER_THAN,
          inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kGreaterOrEqual: {
      binary_node = CreateBinaryOperator<
          DML_ELEMENT_WISE_LOGICAL_GREATER_THAN_OR_EQUAL_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder,
          DML_OPERATOR_ELEMENT_WISE_LOGICAL_GREATER_THAN_OR_EQUAL, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kLesser: {
      binary_node = CreateBinaryOperator<
          DML_ELEMENT_WISE_LOGICAL_LESS_THAN_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_LOGICAL_LESS_THAN, inputs);
      break;
    }
    case mojom::ElementWiseBinary::Kind::kLesserOrEqual: {
      binary_node = CreateBinaryOperator<
          DML_ELEMENT_WISE_LOGICAL_LESS_THAN_OR_EQUAL_OPERATOR_DESC>(
          input_a_tensor_desc, input_b_tensor_desc, output_tensor_desc,
          graph_builder, DML_OPERATOR_ELEMENT_WISE_LOGICAL_LESS_THAN_OR_EQUAL,
          inputs);
      break;
    }
  }
  if (!binary_node) {
    std::string error_message =
        "Failed to create " + OpKindToString(operation->kind) + " operator.";
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        std::move(error_message)));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      binary_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForPad(
    const IdToOperandMap& id_to_operand_map,
    const mojom::PadPtr& pad,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, pad->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = pad->output_operand_id;
  const auto& output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_PADDING_MODE padding_mode;
  // This value is ignored for other padding modes.
  float padding_value = 0;
  switch (pad->mode->which()) {
    case mojom::PaddingMode::Tag::kConstant:
      padding_mode = DML_PADDING_MODE::DML_PADDING_MODE_CONSTANT;
      padding_value = pad->mode->get_constant()->value;
      break;
    case mojom::PaddingMode::Tag::kEdge:
      padding_mode = DML_PADDING_MODE::DML_PADDING_MODE_EDGE;
      break;
    case mojom::PaddingMode::Tag::kReflection:
      padding_mode = DML_PADDING_MODE::DML_PADDING_MODE_REFLECTION;
      break;
    case mojom::PaddingMode::Tag::kSymmetric:
      padding_mode = DML_PADDING_MODE::DML_PADDING_MODE_SYMMETRIC;
      break;
  }

  const auto& beginning_padding = pad->beginning_padding;
  const auto& ending_padding = pad->ending_padding;
  CHECK_EQ(beginning_padding.size(), ending_padding.size());

  DML_PADDING_OPERATOR_DESC pad_operator_desc = {
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .PaddingMode = padding_mode,
      .PaddingValue = padding_value,
      .DimensionCount = static_cast<uint32_t>(beginning_padding.size()),
      .StartPadding = beginning_padding.data(),
      .EndPadding = ending_padding.data()};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* pad_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_PADDING, &pad_operator_desc, {inputs});
  if (!pad_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create pad operator."));
  }

  const NodeOutput* output =
      graph_builder.CreateNodeOutput(pad_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForPool2d(
    const IdToOperandMap& id_to_operand_map,
    const mojom::Pool2dPtr& pool2d,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, pool2d->input_operand_id);
  // The input tensor description may be transposed.
  auto input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = pool2d->output_operand_id;
  // The output tensor description may be transposed.
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  switch (pool2d->layout) {
    case mojom::InputOperandLayout::kChannelsFirst: {
      break;
    }
    // DML pooling operators only support nchw layout according to
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_average_pooling_operator_desc
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_max_pooling2_operator_desc.
    //
    // To support other layouts, we can transpose the input and output tensors
    // to nchw without changing the physical arrangement by modifying the
    // descriptions of dimensions, and strides which determines the number of
    // elements to traverse to reach the next element in each dimension. E.g.,
    // for a tensor with nhwc layout, dimensions [1, 2, 3, 4] and strides [24,
    // 12, 4, 1], the new tensor with nchw layout should be with dimensions [1,
    // 4, 2, 3] and strides [24, 1, 12, 4]. See details in
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_buffer_tensor_desc.
    case mojom::InputOperandLayout::kChannelsLast: {
      input_tensor_desc.Transpose(kNhwcToNchwPermutation);

      // TODO(crbug.com/1476718): Figure out the optimal physical layout for
      // output tensor.
      output_tensor_desc.Transpose(kNhwcToNchwPermutation);
      break;
    }
  }

  std::array<uint32_t, 2> strides = {pool2d->strides->height,
                                     pool2d->strides->width};
  std::array<uint32_t, 2> dilations = {pool2d->dilations->height,
                                       pool2d->dilations->width};
  std::array<uint32_t, 2> window_dimensions = {
      pool2d->window_dimensions->height, pool2d->window_dimensions->width};
  std::array<uint32_t, 2> start_padding = {pool2d->padding->beginning->height,
                                           pool2d->padding->beginning->width};
  std::array<uint32_t, 2> end_padding = {pool2d->padding->ending->height,
                                         pool2d->padding->ending->width};
  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* pool2d_node = nullptr;
  switch (pool2d->kind) {
      // TODO(crbug.com/1273291): Add L2Pool2d operator.

    case mojom::Pool2d::Kind::kAveragePool2d: {
      // TODO(crbug.com/1273291): Work around dilation support for L2 and
      // average pooling. According to WebNN spec:
      // https://www.w3.org/TR/webnn/#api-mlgraphbuilder-pool2d, dilations are
      // supported by pooling operations, while for DirectML AVERAGE_POOLING and
      // LP_POOLING don't support dilations.
      // Spec issue tracked on
      // https://github.com/webmachinelearning/webnn/issues/180.
      if (dilations[0] != 1 || dilations[1] != 1) {
        DLOG(ERROR)
            << "Dilations are not supported for average pooling operator.";
        return base::unexpected(CreateError(
            mojom::Error::Code::kNotSupportedError,
            "Dilations are not supported for average pooling operator."));
      }
      DML_AVERAGE_POOLING_OPERATOR_DESC average_pooling_desc = {
          .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
          .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
          .DimensionCount =
              base::checked_cast<uint32_t>(window_dimensions.size()),
          .Strides = strides.data(),
          .WindowSize = window_dimensions.data(),
          .StartPadding = start_padding.data(),
          .EndPadding = end_padding.data(),
          // The padding elements are not counted as part of the averaging
          // calculation.
          .IncludePadding = false};
      pool2d_node = graph_builder.CreateOperatorNode(
          DML_OPERATOR_AVERAGE_POOLING, &average_pooling_desc, inputs);
      break;
    }
    case mojom::Pool2d::Kind::kL2Pool2d: {
      DML_LP_POOLING_OPERATOR_DESC l2_pooling_desc = {
          .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
          .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
          .DimensionCount =
              base::checked_cast<uint32_t>(window_dimensions.size()),
          .Strides = strides.data(),
          .WindowSize = window_dimensions.data(),
          .StartPadding = start_padding.data(),
          .EndPadding = end_padding.data(),
          .P = 2};
      pool2d_node = graph_builder.CreateOperatorNode(DML_OPERATOR_LP_POOLING,
                                                     &l2_pooling_desc, inputs);
      break;
    }
    case mojom::Pool2d::Kind::kMaxPool2d: {
      // If the dilations are { 1, 1 } by default, prefer using
      // `DML_MAX_POOLING_OPERATOR_DESC` without dilations supported for best
      // compatibility.
      // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_max_pooling_operator_desc.
      // TODO(issues.chromium.org/327244278): Remove the workaround of using
      // `DML_MAX_POOLING_OPERATOR_DESC` without dilations.
      if (dilations[0] == 1 && dilations[1] == 1) {
        DML_MAX_POOLING_OPERATOR_DESC max_pooling_desc = {
            .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
            .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
            .DimensionCount =
                base::checked_cast<uint32_t>(window_dimensions.size()),
            .Strides = strides.data(),
            .WindowSize = window_dimensions.data(),
            .StartPadding = start_padding.data(),
            .EndPadding = end_padding.data()};
        pool2d_node = graph_builder.CreateOperatorNode(
            DML_OPERATOR_MAX_POOLING, &max_pooling_desc, inputs);
      } else {
        DML_MAX_POOLING2_OPERATOR_DESC max_pooling2_desc = {
            .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
            .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
            .OutputIndicesTensor = nullptr,
            .DimensionCount =
                base::checked_cast<uint32_t>(window_dimensions.size()),
            .Strides = strides.data(),
            .WindowSize = window_dimensions.data(),
            .StartPadding = start_padding.data(),
            .EndPadding = end_padding.data(),
            .Dilations = dilations.data()};
        pool2d_node = graph_builder.CreateOperatorNode(
            DML_OPERATOR_MAX_POOLING2, &max_pooling2_desc, inputs);
      }
      break;
    }
    default:
      DLOG(ERROR) << "Invalid Pool2d operator type";
      NOTREACHED_NORETURN();
  }

  if (!pool2d_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create pooling operator."));
  }
  if (pool2d->layout == mojom::InputOperandLayout::kChannelsLast) {
    // Transpose the output tensor from nchw to nhwc layout.
    output_tensor_desc.Transpose(kNchwToNhwcPermutation);
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      pool2d_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForPrelu(
    const IdToOperandMap& id_to_operand_map,
    const mojom::PreluPtr& prelu,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, prelu->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  const NodeOutput* slope =
      GetNodeOutputForOperand(id_to_node_output_map, prelu->slope_operand_id);
  auto slope_tensor_desc = slope->GetTensorDesc();

  uint64_t output_id = prelu->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  const auto& output_dimensions = output_tensor_desc.GetDimensions();
  if (slope_tensor_desc.GetDimensions() != output_dimensions) {
    slope_tensor_desc.BroadcastTo(output_dimensions);
  }

  DML_ACTIVATION_PARAMETERIZED_RELU_OPERATOR_DESC prelu_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .SlopeTensor = &slope_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc()};

  std::array<const NodeOutput*, 2> inputs = {input, slope};
  const OperatorNode* prelu_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_PARAMETERIZED_RELU, &prelu_desc, inputs);
  if (!prelu_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create prelu operator."));
  }

  const NodeOutput* node_output =
      graph_builder.CreateNodeOutput(prelu_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForSlice(
    const IdToOperandMap& id_to_operand_map,
    const mojom::SlicePtr& slice,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  base::expected<void, mojom::ErrorPtr> create_operator_result;
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, slice->input_operand_id);
  const TensorDesc& input_tensor_desc = input->GetTensorDesc();
  const auto& input_dimensions = input_tensor_desc.GetDimensions();

  // Start and size attributes must be unpacked from the mojo interface.
  std::vector<uint32_t> starts;
  std::vector<uint32_t> sizes;
  starts.reserve(slice->starts_and_sizes.size());
  sizes.reserve(slice->starts_and_sizes.size());
  for (size_t i = 0; i < slice->starts_and_sizes.size(); ++i) {
    starts.push_back(slice->starts_and_sizes[i]->start);
    sizes.push_back(slice->starts_and_sizes[i]->size);
  }
  CHECK_EQ(input_dimensions.size(), slice->starts_and_sizes.size());

  const TensorDesc& output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, slice->output_operand_id);

  // WebNN doesn't support the strides parameter, but DML expects one. Create
  // an appropriately sized array of 1s to produce the expected operation.
  std::vector<uint32_t> strides(input_dimensions.size(), 1u);

  DML_SLICE_OPERATOR_DESC slice_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .DimensionCount = static_cast<UINT>(input_dimensions.size()),
      .Offsets = starts.data(),
      .Sizes = sizes.data(),
      .Strides = strides.data(),
  };
  std::array<const NodeOutput*, 1> input_node_output = {input};
  const OperatorNode* slice_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_SLICE, &slice_operator_desc, input_node_output);
  if (!slice_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create slice operator."));
  }

  const auto* slice_output =
      graph_builder.CreateNodeOutput(slice_node, std::move(output_tensor_desc));
  id_to_node_output_map[slice->output_operand_id] = std::move(slice_output);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForSplit(
    const IdToOperandMap& id_to_operand_map,
    const mojom::SplitPtr& split,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, split->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  // Since TensorDesc stores dimensions and strides vectors, we need to keep
  // TensorDescs until create CreateOperatorNode is called.
  std::vector<TensorDesc> output_tensor_desc;
  output_tensor_desc.reserve(split->output_operand_ids.size());
  std::vector<DML_TENSOR_DESC> output_tensor_desc_dml;
  output_tensor_desc_dml.reserve(output_tensor_desc.size());
  for (uint64_t output_id : split->output_operand_ids) {
    output_tensor_desc.push_back(
        CreateOutputTensorDesc(id_to_operand_map, output_id));
    output_tensor_desc_dml.push_back(
        output_tensor_desc.back().GetDMLTensorDesc());
  }

  auto output_count =
      base::checked_cast<uint32_t>(output_tensor_desc_dml.size());
  DML_SPLIT_OPERATOR_DESC split_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputCount = output_count,
      .OutputTensors = output_tensor_desc_dml.data(),
      .Axis = split->axis};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* split_node =
      graph_builder.CreateOperatorNode(DML_OPERATOR_SPLIT, &split_desc, inputs);

  if (!split_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create split operator."));
  }

  for (uint32_t i = 0; i < output_count; ++i) {
    uint64_t output_id = split->output_operand_ids[i];
    const auto* output = graph_builder.CreateNodeOutput(
        split_node, std::move(output_tensor_desc[i]), i);
    CHECK(id_to_node_output_map.try_emplace(output_id, output).second);
  }

  return base::ok();
}

template <typename DML_OPERATOR_DESC, DML_OPERATOR_TYPE operator_type>
const OperatorNode* CreateUnaryOperator(const TensorDesc& input_tensor,
                                        const TensorDesc& output_tensor,
                                        const NodeOutput* input,
                                        GraphBuilder& graph_builder) {
  DML_OPERATOR_DESC unary_operator_desc{
      .InputTensor = &input_tensor.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor.GetDMLTensorDesc()};
  std::array<const NodeOutput*, 1> inputs = {input};
  return graph_builder.CreateOperatorNode(operator_type, &unary_operator_desc,
                                          inputs);
}

template <typename OperatorDesc,
          DML_OPERATOR_TYPE operator_type,
          typename Operation>
base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForUnary(
    const IdToOperandMap& id_to_operand_map,
    const Operation& operation,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, operation->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = operation->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  const OperatorNode* unary_node =
      CreateUnaryOperator<OperatorDesc, operator_type>(
          input_tensor_desc, output_tensor_desc, input, graph_builder);
  if (!unary_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create unary operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      unary_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForNeg(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ElementWiseUnaryPtr& operation,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, operation->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  const uint64_t output_id = operation->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  // Set the values of scale and bias terms supplied to identity operator. Scale
  // and bias have the effect of applying the function g(x) = x * Scale + Bias.
  // When we set Scale to -1 and Bias to 0, we can simulate identity as negate
  // operator.
  DML_SCALE_BIAS scale_bias{.Scale = -1.f, .Bias = 0.f};
  DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC identity_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .ScaleBias = &scale_bias};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* identity_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ELEMENT_WISE_IDENTITY, &identity_operator_desc, inputs);
  if (!identity_node) {
    return base::unexpected(
        mojom::Error::New(mojom::Error::Code::kUnknownError,
                          "Failed to create identity operator to implement "
                          "WebNN neg operation."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      identity_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForElementWiseUnary(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ElementWiseUnaryPtr& operation,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  switch (operation->kind) {
    case mojom::ElementWiseUnary::Kind::kAbs: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_ABS_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_ABS>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kCeil: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_CEIL_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_CEIL>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kCos: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_COS_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_COS>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kExp: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_EXP_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_EXP>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kFloor: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_FLOOR_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_FLOOR>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kLog: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_LOG_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_LOG>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    // TODO(crbug.com/1502731): Implement the negate operator directly by
    // DML_ELEMENT_WISE_NEGATE_OPERATOR_DESC which is available in
    // DML_FEATURE_LEVEL_5_0.
    // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_element_wise_negate_operator_desc#availability
    case mojom::ElementWiseUnary::Kind::kNeg: {
      return CreateOperatorNodeForNeg(id_to_operand_map, operation,
                                      graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kSin: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_SIN_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_SIN>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kTan: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_TAN_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_TAN>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kLogicalNot: {
      return CreateOperatorNodeForUnary<
          DML_ELEMENT_WISE_LOGICAL_NOT_OPERATOR_DESC,
          DML_OPERATOR_ELEMENT_WISE_LOGICAL_NOT>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kIdentity: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_IDENTITY>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kSqrt: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_SQRT_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_SQRT>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kErf: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_ERF_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_ERF>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kReciprocal: {
      return CreateOperatorNodeForUnary<DML_ELEMENT_WISE_RECIP_OPERATOR_DESC,
                                        DML_OPERATOR_ELEMENT_WISE_RECIP>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
    case mojom::ElementWiseUnary::Kind::kCast: {
      return CreateOperatorNodeForUnary<DML_CAST_OPERATOR_DESC,
                                        DML_OPERATOR_CAST>(
          id_to_operand_map, operation, graph_builder, id_to_node_output_map);
    }
  }
  NOTREACHED_NORETURN();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForResample2d(
    const IdToOperandMap& id_to_operand_map,
    const mojom::Resample2dPtr& resample2d,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, resample2d->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = resample2d->output_operand_id;
  const auto& output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  const auto& input_dimensions = input_tensor_desc.GetDimensions();
  const auto& output_dimensions = output_tensor_desc.GetDimensions();
  size_t input_rank = input_dimensions.size();
  CHECK_EQ(input_rank, output_dimensions.size());

  // Use explicit scales if given, otherwise, compute scales from output
  // dimensions / input dimensions. Then expand scales to full scales (same size
  // as input rank using axes).
  std::vector<float> full_scales(input_rank, 1);
  const auto& scales = resample2d->scales;
  const auto& axes = resample2d->axes;
  if (scales) {
    for (size_t i = 0; i < axes.size(); ++i) {
      auto axis = axes[i];
      CHECK_LT(axis, full_scales.size());
      full_scales[axis] = scales.value()[i];
    }
  } else {
    for (size_t i = 0; i < input_rank; ++i) {
      full_scales[i] =
          base::checked_cast<float>(output_dimensions[i]) / input_dimensions[i];
    }
  }

  DML_INTERPOLATION_MODE mode;
  switch (resample2d->mode) {
    case mojom::Resample2d::InterpolationMode::kNearestNeighbor:
      mode = DML_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
      break;
    case mojom::Resample2d::InterpolationMode::kLinear:
      mode = DML_INTERPOLATION_MODE_LINEAR;
      break;
  }

  DML_RESAMPLE_OPERATOR_DESC resample2d_operator_desc = {
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .InterpolationMode = mode,
      .ScaleCount = static_cast<uint32_t>(full_scales.size()),
      .Scales = full_scales.data()};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* resample2d_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_RESAMPLE, &resample2d_operator_desc, inputs);
  if (!resample2d_node) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "Failed to create resample2d operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      resample2d_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForReduce(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ReducePtr& reduce,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, reduce->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  uint64_t output_id = reduce->output_operand_id;
  const auto& output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  const auto& axes = reduce->axes;
  // Determine output sizes. Ignore output_desc->dimensions for the dimensions,
  // since DirectML expects the output dimensions to have the same rank as the
  // input, and output_desc->dimensions may have removed dimensions if
  // keepDimensions was false.
  std::vector<uint32_t> output_dimensions = input_tensor_desc.GetDimensions();
  for (uint32_t axis : axes) {
    CHECK_LT(axis, output_dimensions.size());
    output_dimensions[axis] = 1u;
  }
  TensorDesc new_output_tensor_desc(output_tensor_desc.GetDataType(),
                                    output_dimensions);

  std::array<const NodeOutput*, 1> inputs = {input};
  DML_REDUCE_OPERATOR_DESC operator_desc = {};
  operator_desc.Function = MapReduceKindToReduceFuntion(reduce->kind);
  operator_desc.InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
  operator_desc.OutputTensor = &new_output_tensor_desc.GetDMLTensorDesc(),
  operator_desc.AxisCount = static_cast<uint32_t>(axes.size());
  operator_desc.Axes = axes.data();
  const OperatorNode* reduce_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_REDUCE, &operator_desc, inputs);
  if (!reduce_node) {
    std::string error_message =
        "Failed to create " + OpKindToString(reduce->kind) + " operator.";
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        std::move(error_message)));
  }

  const NodeOutput* output =
      graph_builder.CreateNodeOutput(reduce_node, output_tensor_desc);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

// DirectML API does not have a real Reshape operator. The WebNN Reshape is
// implemented by creating a new NodeOutput for the input Node. The new
// NodeOutput has the reshaped dimensions and is used as the output of the WebNN
// Reshape operator. And if the input and output of the Reshape are exactly the
// input and output of the DirectML graph, we need to add another DirectML
// Identity operator to ensure that the DirectML graph can be compiled and
// calculated correctly.
void CreateNodeOutputForReshape(const IdToOperandMap& id_to_operand_map,
                                const mojom::ReshapePtr& reshape,
                                GraphBuilder& graph_builder,
                                IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, reshape->input_operand_id);

  // Ensure the output tensor description having the
  // `DML_TENSOR_FLAG_OWNED_BY_DML` flag if its corresponding node is a constant
  // graph input.
  uint64_t output_id = reshape->output_operand_id;
  const OperandPtr& output_operand = id_to_operand_map.at(output_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  auto output_tensor_desc =
      TensorDesc(input_tensor_desc.GetDataType(), input_tensor_desc.GetFlags(),
                 output_operand->dimensions);

  const Node& input_node = input->GetNode();

  // The output_index of this NodeOutput should be the same as the input
  // NodeOutput for creating correct intermediate edges of the graph.
  const NodeOutput* output = graph_builder.CreateNodeOutput(
      &input_node, std::move(output_tensor_desc), input->GetOutputIndex());

  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForElu(
    const IdToOperandMap& id_to_operand_map,
    const mojom::EluPtr& elu,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, elu->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = elu->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_ACTIVATION_ELU_OPERATOR_DESC elu_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Alpha = elu->alpha};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* elu_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_ELU, &elu_desc, inputs);
  if (!elu_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create elu operator."));
  }

  const NodeOutput* node_output =
      graph_builder.CreateNodeOutput(elu_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForExpand(
    const IdToOperandMap& id_to_operand_map,
    const mojom::ExpandPtr& expand,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, expand->input_operand_id);
  auto input_tensor_desc = input->GetTensorDesc();

  const uint64_t output_id = expand->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  // Use identity to implement the expand operation with broadcasting strides
  // https://learn.microsoft.com/en-us/windows/ai/directml/dml-strides#broadcasting-with-strides.
  const auto& output_dimensions = output_tensor_desc.GetDimensions();
  if (input_tensor_desc.GetDimensions() != output_dimensions) {
    input_tensor_desc.BroadcastTo(output_dimensions);
  }
  const OperatorNode* identity_node =
      CreateUnaryOperator<DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC,
                          DML_OPERATOR_ELEMENT_WISE_IDENTITY>(
          input_tensor_desc, output_tensor_desc, input, graph_builder);
  if (!identity_node) {
    return base::unexpected(
        mojom::Error::New(mojom::Error::Code::kUnknownError,
                          "Failed to create identity dml operator to implement "
                          "expand operation."));
  }

  const NodeOutput* node_output = graph_builder.CreateNodeOutput(
      identity_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForGather(
    const IdToOperandMap& id_to_operand_map,
    const mojom::GatherPtr& gather,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, gather->input_operand_id);
  auto input_tensor_desc = input->GetTensorDesc();

  const NodeOutput* indices = GetNodeOutputForOperand(
      id_to_node_output_map, gather->indices_operand_id);
  auto indices_tensor_desc = indices->GetTensorDesc();
  size_t indices_rank = indices_tensor_desc.GetDimensions().size();
  if (!base::MakeCheckedNum(indices_rank).IsValid<uint32_t>()) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "The indices rank of gather operator is too large."));
  }

  uint64_t output_id = gather->output_operand_id;
  const auto original_output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  auto output_tensor_desc = original_output_tensor_desc;

  size_t input_rank = input_tensor_desc.GetDimensions().size();
  size_t output_rank = output_tensor_desc.GetDimensions().size();
  size_t expanded_rank = std::max(input_rank, output_rank);

  // According to the DirectML documentation
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_gather_operator_desc,
  // the parameters `InputTensor`, `OutputTensor` and `IndicesTensor` must have
  // the same dimension count.
  input_tensor_desc.EnsureMinimumRank(expanded_rank,
                                      TensorDesc::Alignment::kTrailing);
  indices_tensor_desc.EnsureMinimumRank(expanded_rank,
                                        TensorDesc::Alignment::kTrailing);

  uint32_t axis = gather->axis;
  if (output_rank < input_rank) {
    // There is only one case in which `output_rank` is less than `input_rank`,
    // that is when indices is scalar. In this case, a one value should be
    // inserted at the `axis` position of the output dimensions, because the
    // indices dimensions is set to {1} since DirectML requires the tensor
    // dimension count to be at least 1.
    CHECK_EQ(indices_rank, 1u);
    CHECK_EQ(output_rank, input_rank - 1);

    auto output_dimensions = input_tensor_desc.GetDimensions();
    CHECK_LT(axis, output_dimensions.size());
    output_dimensions[axis] = 1;
    output_tensor_desc = TensorDesc(output_tensor_desc.GetDataType(),
                                    std::move(output_dimensions));
  }

  auto expanded_axis = base::MakeCheckedNum(expanded_rank) - input_rank +
                       base::checked_cast<size_t>(axis);
  if (!expanded_axis.AssignIfValid<uint32_t>(&axis)) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "The axis of gather operator is too large."));
  }

  // TODO(crbug.com/1273291): Include a DirectML documentation link and a
  // Chromium test that validates the out-of-bounds indices handling.
  //
  // DirectML implementation for gather operator has already handled the
  // indices tensor by clamping it in the shader to prevent out-of-bounds
  // access.
  DML_GATHER_OPERATOR_DESC gather_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .IndicesTensor = &indices_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      // The axis dimension of InputTensor to gather on.
      .Axis = axis,
      // The number of actual index dimensions within the IndicesTensor.
      .IndexDimensions = base::checked_cast<uint32_t>(indices_rank)};

  std::array<const NodeOutput*, 2> inputs = {input, indices};
  const OperatorNode* gather_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_GATHER, &gather_operator_desc, inputs);
  if (!gather_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create gather operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      gather_node, std::move(original_output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

// Creates a DirectML operator for the WebNN general matrix multiplication
// (GEMM) of the expression alpha * A * B + beta * C.
base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForGemm(
    const IdToOperandMap& id_to_operand_map,
    const mojom::GemmPtr& gemm,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input_a_node_output =
      GetNodeOutputForOperand(id_to_node_output_map, gemm->a_operand_id);
  auto input_a_tensor_desc = input_a_node_output->GetTensorDesc();

  const NodeOutput* input_b_node_output =
      GetNodeOutputForOperand(id_to_node_output_map, gemm->b_operand_id);
  auto input_b_tensor_desc = input_b_node_output->GetTensorDesc();

  std::vector<const NodeOutput*> inputs{input_a_node_output,
                                        input_b_node_output};

  uint64_t output_id = gemm->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  // The input c tensor description may be broadcasted.
  std::optional<TensorDesc> input_c_tensor_desc;
  auto& c_operand_id = gemm->c_operand_id;
  if (c_operand_id) {
    uint64_t input_c_id = c_operand_id.value();

    const auto input_c_node_output_iterator =
        id_to_node_output_map.find(input_c_id);
    CHECK(input_c_node_output_iterator != id_to_node_output_map.end());

    const NodeOutput* input_c_node_output =
        input_c_node_output_iterator->second;
    CHECK(input_c_node_output);
    input_c_tensor_desc = input_c_node_output->GetTensorDesc();

    // Ensure the graph edge for c operand will be created.
    inputs.push_back(input_c_node_output);

    auto output_dimensions = output_tensor_desc.GetDimensions();
    if (input_c_tensor_desc->GetDimensions() != output_dimensions) {
      input_c_tensor_desc->BroadcastTo(output_dimensions);
    }
  }

  // Use 4D GEMM which is available since feature level 1.0 for best
  // compatibility. There is no performance difference in the shader between
  // 2D/3D/4D, as 2D is just a variant of 4D with a batch/channel size of 1.
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_gemm_operator_desc.
  // TODO(issues.chromium.org/327244277): Remove the workaround of coercing
  // GEMM's tensors to 4D.
  input_a_tensor_desc.EnsureMinimumRank(4, TensorDesc::Alignment::kTrailing);
  input_b_tensor_desc.EnsureMinimumRank(4, TensorDesc::Alignment::kTrailing);
  if (input_c_tensor_desc) {
    input_c_tensor_desc->EnsureMinimumRank(4, TensorDesc::Alignment::kTrailing);
  }
  auto expanded_output_tensor_desc = output_tensor_desc;
  expanded_output_tensor_desc.EnsureMinimumRank(
      4, TensorDesc::Alignment::kTrailing);

  DML_GEMM_OPERATOR_DESC gemm_operator_desc{
      .ATensor = &input_a_tensor_desc.GetDMLTensorDesc(),
      .BTensor = &input_b_tensor_desc.GetDMLTensorDesc(),
      .CTensor = input_c_tensor_desc.has_value()
                     ? &input_c_tensor_desc->GetDMLTensorDesc()
                     : nullptr,
      .OutputTensor = &expanded_output_tensor_desc.GetDMLTensorDesc(),
      .TransA = (gemm->a_transpose) ? DML_MATRIX_TRANSFORM_TRANSPOSE
                                    : DML_MATRIX_TRANSFORM_NONE,
      .TransB = (gemm->b_transpose) ? DML_MATRIX_TRANSFORM_TRANSPOSE
                                    : DML_MATRIX_TRANSFORM_NONE,
      .Alpha = gemm->alpha,
      .Beta = gemm->beta,
      .FusedActivation = nullptr,  // Not supported
  };

  const OperatorNode* gemm_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_GEMM, &gemm_operator_desc, inputs);
  if (!gemm_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create gemm operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      gemm_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForHardSigmoid(
    const IdToOperandMap& id_to_operand_map,
    const mojom::HardSigmoidPtr& hard_sigmoid,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, hard_sigmoid->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  const uint64_t output_id = hard_sigmoid->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_ACTIVATION_HARD_SIGMOID_OPERATOR_DESC hard_sigmoid_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Alpha = hard_sigmoid->alpha,
      .Beta = hard_sigmoid->beta};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* hard_sigmoid_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_HARD_SIGMOID, &hard_sigmoid_desc, inputs);
  if (!hard_sigmoid_node) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "Failed to create hard sigmoid operator."));
  }

  const NodeOutput* node_output = graph_builder.CreateNodeOutput(
      hard_sigmoid_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

// Since DirectML feature levels before 6.2, we need to implement hardSwish
// by composing from smaller operators:
// Output = input * clamp((input / 6) + 0.5, 0, 1).
base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForHardSwish(
    const IdToOperandMap& id_to_operand_map,
    const mojom::HardSwishPtr& hard_swish,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, hard_swish->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  const uint64_t output_id = hard_swish->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  // First step: build `clamp((x / 6) + 0.5, 0, 1)`.
  DML_SCALE_BIAS scale_bias = {.Scale = 1.0 / 6.0, .Bias = 0.5};
  DML_ELEMENT_WISE_CLIP_OPERATOR_DESC clamp_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      // Applying the function `g(x) = x / 6 + 0.5` to each input element prior
      // to clamp.
      .ScaleBias = &scale_bias,
      .Min = 0,
      .Max = 1};
  std::array<const NodeOutput*, 1> clamp_inputs = {input};
  const OperatorNode* clamp_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ELEMENT_WISE_CLIP, &clamp_operator_desc, clamp_inputs);
  if (!clamp_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create clamp operator."));
  }
  const NodeOutput* clamp_output =
      graph_builder.CreateNodeOutput(clamp_node, output_tensor_desc, 0);
  const auto& clamp_output_tensor_desc = clamp_output->GetTensorDesc();

  // Second step: build `x * first_step`.
  std::array<const NodeOutput*, 2> mul_inputs = {input, clamp_output};
  DML_ELEMENT_WISE_MULTIPLY_OPERATOR_DESC binary_mul_desc{
      .ATensor = &input_tensor_desc.GetDMLTensorDesc(),
      .BTensor = &clamp_output_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc()};
  const OperatorNode* binary_mul_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ELEMENT_WISE_MULTIPLY, &binary_mul_desc, mul_inputs);
  if (!binary_mul_node) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "Failed to create binary mul operator."));
  }
  const NodeOutput* mul_output =
      graph_builder.CreateNodeOutput(binary_mul_node, output_tensor_desc);

  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, mul_output).second);

  return base::ok();
}

template <typename NormalizationPtr>
base::expected<void, mojom::ErrorPtr>
CreateOperatorNodeForMeanVarianceNormalization(
    const NormalizationPtr& normalization,
    mojom::GraphInfoPtr& graph_info,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map,
    std::unordered_map<uint64_t, uint32_t>& constant_id_to_input_index_map,
    uint64_t& next_operand_id,
    base::span<const uint32_t> mean_variance_axes,
    base::span<const uint32_t> scale_bias_broadcast_axes,
    mojom::Operation::Tag op) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, normalization->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();
  size_t input_rank = input_tensor_desc.GetDimensions().size();

  auto& id_to_operand_map = graph_info->id_to_operand_map;
  uint64_t output_id = normalization->output_operand_id;
  const OperandPtr& output_operand = id_to_operand_map.at(output_id);
  Operand::DataType data_type = output_operand->data_type;
  const TensorDesc output_tensor_desc(GetTensorDataType(data_type),
                                      output_operand->dimensions);

  const NodeOutput* scale =
      normalization->scale_operand_id.has_value()
          ? GetNodeOutputForOperand(id_to_node_output_map,
                                    normalization->scale_operand_id.value())
          : nullptr;
  const NodeOutput* bias =
      normalization->bias_operand_id.has_value()
          ? GetNodeOutputForOperand(id_to_node_output_map,
                                    normalization->bias_operand_id.value())
          : nullptr;

  // DML_MEAN_VARIANCE_NORMALIZATION1_OPERATOR_DESC requires `ScaleTensor` and
  // `BiasTensor` to be both present or not present when DML_FEATURE_LEVEL is
  // less than DML_FEATURE_LEVEL_5_2.
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_mean_variance_normalization1_operator_desc.
  //
  // If one of scale/bias is not present, create a constant operand for it and
  // insert the operand into the graph.
  if ((scale && !bias) || (!scale && bias)) {
    if (!scale) {
      uint64_t scale_operand_id = BuildConstantOperandForFloatValue(
          graph_info, next_operand_id, data_type,
          scale_bias_broadcast_axes.size(),
          /*default scale*/ 1.0);

      // Create an input node for the scale operand and store the assigned input
      // index in `constant_id_to_input_index_map`, which will be used for
      // constant buffer binding.
      uint32_t scale_input_index =
          CreateInputNode(id_to_operand_map, scale_operand_id, graph_builder,
                          id_to_node_output_map);
      CHECK(constant_id_to_input_index_map
                .try_emplace(scale_operand_id, scale_input_index)
                .second);

      scale = GetNodeOutputForOperand(id_to_node_output_map, scale_operand_id);
    }
    if (!bias) {
      uint64_t bias_operand_id = BuildConstantOperandForFloatValue(
          graph_info, next_operand_id, data_type,
          scale_bias_broadcast_axes.size(),
          /*default bias*/ 0);

      // Create an input node for the bias operand and store the assigned input
      // index in `constant_id_to_input_index_map`, which will be used for
      // constant buffer binding.
      uint32_t bias_input_index =
          CreateInputNode(id_to_operand_map, bias_operand_id, graph_builder,
                          id_to_node_output_map);
      CHECK(constant_id_to_input_index_map
                .try_emplace(bias_operand_id, bias_input_index)
                .second);

      bias = GetNodeOutputForOperand(id_to_node_output_map, bias_operand_id);
    }
  }

  if (!base::MakeCheckedNum(mean_variance_axes.size()).IsValid<uint32_t>()) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    OpTagToString(op) + ": The axes rank is too large."));
  }

  std::vector<const NodeOutput*> inputs = {input};
  std::optional<TensorDesc> scale_tensor_desc;
  std::optional<TensorDesc> bias_tensor_desc;

  if (scale) {
    inputs.push_back(scale);
    scale_tensor_desc = scale->GetTensorDesc();
    // The scale tensor should have the same rank as the input tensor required
    // by DML_MEAN_VARIANCE_NORMALIZATION1_OPERATOR_DESC.
    scale_tensor_desc->MakeBroadcastCompatible(input_rank,
                                               scale_bias_broadcast_axes);
  }
  if (bias) {
    inputs.push_back(bias);
    bias_tensor_desc = bias->GetTensorDesc();
    // The bias tensor should have the same rank as the input tensor required by
    // DML_MEAN_VARIANCE_NORMALIZATION1_OPERATOR_DESC.
    bias_tensor_desc->MakeBroadcastCompatible(input_rank,
                                              scale_bias_broadcast_axes);
  }

  DML_MEAN_VARIANCE_NORMALIZATION1_OPERATOR_DESC
  normalization_operator_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .ScaleTensor = scale_tensor_desc.has_value()
                         ? &scale_tensor_desc->GetDMLTensorDesc()
                         : nullptr,
      .BiasTensor = bias_tensor_desc.has_value()
                        ? &bias_tensor_desc->GetDMLTensorDesc()
                        : nullptr,
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .AxisCount = base::checked_cast<uint32_t>(mean_variance_axes.size()),
      .Axes = mean_variance_axes.data(),
      // The layer normalization and instance normalization includes variance.
      .NormalizeVariance = true,
      .Epsilon = normalization->epsilon,
      .FusedActivation = nullptr};

  const OperatorNode* normalization_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_MEAN_VARIANCE_NORMALIZATION1, &normalization_operator_desc,
      inputs);
  if (!normalization_node) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    base::StringPrintf("Failed to create %s operator.",
                                       OpTagToString(op).c_str())));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      normalization_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForLeakyRelu(
    const IdToOperandMap& id_to_operand_map,
    const mojom::LeakyReluPtr& leaky_relu,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, leaky_relu->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = leaky_relu->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_ACTIVATION_LEAKY_RELU_OPERATOR_DESC leaky_relu_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Alpha = leaky_relu->alpha};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* leaky_relu_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_LEAKY_RELU, &leaky_relu_desc, inputs);
  if (!leaky_relu_node) {
    return base::unexpected(
        CreateError(mojom::Error::Code::kUnknownError,
                    "Failed to create leakyRelu operator."));
  }

  const NodeOutput* node_output = graph_builder.CreateNodeOutput(
      leaky_relu_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForLinear(
    const IdToOperandMap& id_to_operand_map,
    const mojom::LinearPtr& linear,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input =
      GetNodeOutputForOperand(id_to_node_output_map, linear->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = linear->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  DML_ACTIVATION_LINEAR_OPERATOR_DESC linear_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Alpha = linear->alpha,
      .Beta = linear->beta};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* linear_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_LINEAR, &linear_desc, inputs);
  if (!linear_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create linear operator."));
  }

  const NodeOutput* node_output = graph_builder.CreateNodeOutput(
      linear_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

// Using DML_GEMM_OPERATOR_DESC to implement WebNN matmul.
base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForMatmul(
    const IdToOperandMap& id_to_operand_map,
    const mojom::MatmulPtr& matmul,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input_a_node_output =
      GetNodeOutputForOperand(id_to_node_output_map, matmul->a_operand_id);
  auto input_a_tensor_desc = input_a_node_output->GetTensorDesc();
  const NodeOutput* input_b_node_output =
      GetNodeOutputForOperand(id_to_node_output_map, matmul->b_operand_id);
  auto input_b_tensor_desc = input_b_node_output->GetTensorDesc();

  uint64_t output_id = matmul->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  const auto output_tensor_dims = output_tensor_desc.GetDimensions();
  // Because DML_GEMM_OPERATOR_DESC restricts input_a_tensor and input_b_tensor,
  // output_tensor must have the same DimensionCount and can't support
  // broadcasting, input_a_tensor and input_b_tensor may need to be broadcasted.
  if (output_tensor_dims.size() > 2) {
    input_a_tensor_desc.BroadcastTo(output_tensor_dims, 2);
    input_b_tensor_desc.BroadcastTo(output_tensor_dims, 2);
  }

  CHECK_EQ(input_a_tensor_desc.GetDimensions().size(),
           input_b_tensor_desc.GetDimensions().size());
  CHECK_EQ(input_a_tensor_desc.GetDimensions().size(),
           output_tensor_dims.size());

  // Use 4D GEMM which is available since feature level 1.0 for best
  // compatibility. There is no performance difference in the shader between
  // 2D/3D/4D, as 2D is just a variant of 4D with a batch/channel size of 1.
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_gemm_operator_desc.
  // TODO(issues.chromium.org/327244277): Remove the workaround of coercing
  // GEMM's tensors to 4D.
  auto expanded_output_tensor_desc = output_tensor_desc;
  if (output_tensor_dims.size() < 4) {
    input_a_tensor_desc.EnsureMinimumRank(4, TensorDesc::Alignment::kTrailing);
    input_b_tensor_desc.EnsureMinimumRank(4, TensorDesc::Alignment::kTrailing);
    expanded_output_tensor_desc.EnsureMinimumRank(
        4, TensorDesc::Alignment::kTrailing);
  }

  DML_GEMM_OPERATOR_DESC matmul_operator_desc{
      .ATensor = &input_a_tensor_desc.GetDMLTensorDesc(),
      .BTensor = &input_b_tensor_desc.GetDMLTensorDesc(),
      .CTensor = nullptr,
      .OutputTensor = &expanded_output_tensor_desc.GetDMLTensorDesc(),
      .TransA = DML_MATRIX_TRANSFORM_NONE,
      .TransB = DML_MATRIX_TRANSFORM_NONE,
      .Alpha = 1.0f,
      .Beta = 0.0f,
      .FusedActivation = nullptr,
  };

  std::array<const NodeOutput*, 2> inputs{input_a_node_output,
                                          input_b_node_output};
  const OperatorNode* matmul_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_GEMM, &matmul_operator_desc, inputs);
  if (!matmul_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create matmul operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      matmul_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForSoftplus(
    const IdToOperandMap& id_to_operand_map,
    const mojom::SoftplusPtr& softplus,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(id_to_node_output_map,
                                                    softplus->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  const uint64_t output_id = softplus->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);

  // The steepness must be greater than or equal to 1.0 when DML_FEATURE_LEVEL
  // is less than DML_FEATURE_LEVEL_6_3:
  // https://learn.microsoft.com/en-us/windows/win32/api/directml/ns-directml-dml_activation_softplus_operator_desc
  if (softplus->steepness < 1.0f) {
    return base::unexpected(CreateError(
        mojom::Error::Code::kNotSupportedError,
        "The steepness of softplus should be greater than or equal to 1.0."));
  }

  DML_ACTIVATION_SOFTPLUS_OPERATOR_DESC softplus_desc{
      .InputTensor = &input_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc(),
      .Steepness = softplus->steepness};

  std::array<const NodeOutput*, 1> inputs = {input};
  const OperatorNode* softplus_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ACTIVATION_SOFTPLUS, &softplus_desc, inputs);
  if (!softplus_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create softplus operator."));
  }

  const NodeOutput* node_output = graph_builder.CreateNodeOutput(
      softplus_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, node_output).second);

  return base::ok();
}

// Transpose is not a real DirectML operator. As for implementation, the input
// tensor is remapped for reading elements following the strides after the
// permutation, and an identity operator is appended to consume the remapped
// strides.
base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForTranspose(
    const IdToOperandMap& id_to_operand_map,
    const mojom::TransposePtr& transpose,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* input = GetNodeOutputForOperand(
      id_to_node_output_map, transpose->input_operand_id);
  const auto& input_tensor_desc = input->GetTensorDesc();

  uint64_t output_id = transpose->output_operand_id;
  auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  CHECK_EQ(input_tensor_desc.GetDimensions().size(),
           output_tensor_desc.GetDimensions().size());

  TensorDesc remapped_input_tensor_desc = input_tensor_desc;
  remapped_input_tensor_desc.Transpose(transpose->permutation);

  // Append an identity node to consume the strides.
  const OperatorNode* identity_node =
      CreateUnaryOperator<DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC,
                          DML_OPERATOR_ELEMENT_WISE_IDENTITY>(
          remapped_input_tensor_desc, output_tensor_desc, input, graph_builder);
  if (!identity_node) {
    return base::unexpected(CreateError(mojom::Error::Code::kUnknownError,
                                        "Failed to create identity operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      identity_node, std::move(output_tensor_desc));
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

base::expected<void, mojom::ErrorPtr> CreateOperatorNodeForWhere(
    const IdToOperandMap& id_to_operand_map,
    const mojom::WherePtr& where,
    GraphBuilder& graph_builder,
    IdToNodeOutputMap& id_to_node_output_map) {
  const NodeOutput* condition = GetNodeOutputForOperand(
      id_to_node_output_map, where->condition_operand_id);
  auto condition_tensor_desc = condition->GetTensorDesc();

  const NodeOutput* true_value = GetNodeOutputForOperand(
      id_to_node_output_map, where->true_value_operand_id);
  auto true_value_tensor_desc = true_value->GetTensorDesc();

  const NodeOutput* false_value = GetNodeOutputForOperand(
      id_to_node_output_map, where->false_value_operand_id);
  auto false_value_tensor_desc = false_value->GetTensorDesc();

  uint64_t output_id = where->output_operand_id;
  const auto output_tensor_desc =
      CreateOutputTensorDesc(id_to_operand_map, output_id);
  const auto output_tensor_dims = output_tensor_desc.GetDimensions();

  // Broadcast each of the inputs to the output.
  if (condition_tensor_desc.GetDimensions() != output_tensor_dims) {
    condition_tensor_desc.BroadcastTo(output_tensor_dims);
  }
  if (true_value_tensor_desc.GetDimensions() != output_tensor_dims) {
    true_value_tensor_desc.BroadcastTo(output_tensor_dims);
  }
  if (false_value_tensor_desc.GetDimensions() != output_tensor_dims) {
    false_value_tensor_desc.BroadcastTo(output_tensor_dims);
  }

  DML_ELEMENT_WISE_IF_OPERATOR_DESC where_operator_desc{
      .ConditionTensor = &condition_tensor_desc.GetDMLTensorDesc(),
      .ATensor = &true_value_tensor_desc.GetDMLTensorDesc(),
      .BTensor = &false_value_tensor_desc.GetDMLTensorDesc(),
      .OutputTensor = &output_tensor_desc.GetDMLTensorDesc()};

  std::array<const NodeOutput*, 3> inputs{condition, true_value, false_value};
  const OperatorNode* where_node = graph_builder.CreateOperatorNode(
      DML_OPERATOR_ELEMENT_WISE_IF, &where_operator_desc, inputs);
  if (!where_node) {
    return base::unexpected(mojom::Error::New(
        mojom::Error::Code::kUnknownError, "Failed to create where operator."));
  }

  const NodeOutput* output = graph_builder.CreateNodeOutput(
      where_node, std::move(output_tensor_desc), 0);
  // The output id must be unique in the map.
  CHECK(id_to_node_output_map.try_emplace(output_id, output).second);

  return base::ok();
}

// If graph creation fails, log the error message and report it.
void HandleGraphCreationFailure(
    const std::string& error_message,
    mojom::WebNNContext::CreateGraphCallback callback) {
  DLOG(ERROR) << error_message;
  std::move(callback).Run(CreateGraphResult::NewError(
      CreateError(mojom::Error::Code::kUnknownError, error_message)));
}

// Similar to the method above, if graph creation fails, report the error
// message and log it with the system error code `hr`. In addition, log and
// report the out of memory error message if there is.
void HandleGraphCreationFailure(
    const std::string& error_message,
    HRESULT hr,
    mojom::WebNNContext::CreateGraphCallback callback) {
  DLOG(ERROR) << error_message << " " << logging::SystemErrorCodeToString(hr);
  if (hr == E_OUTOFMEMORY) {
    DLOG(ERROR) << "No enough memory resources are available.";
    std::move(callback).Run(CreateGraphResult::NewError(CreateError(
        mojom::Error::Code::kUnknownError,
        error_message + " No enough memory resources are available.")));
  } else {
    std::move(callback).Run(CreateGraphResult::NewError(
        CreateError(mojom::Error::Code::kUnknownError, error_message)));
  }
}

}  // namespace

GraphImpl::GraphBufferBindingInfo::GraphBufferBindingInfo() = default;
GraphImpl::GraphBufferBindingInfo::~GraphBufferBindingInfo() = default;

GraphImpl::GraphBufferBindingInfo::GraphBufferBindingInfo(
    GraphBufferBindingInfo&&) = default;
GraphImpl::GraphBufferBindingInfo& GraphImpl::GraphBufferBindingInfo::operator=(
    GraphBufferBindingInfo&&) = default;

GraphImpl::PersistentResource::PersistentResource(
    uint64_t persistent_buffer_byte_length,
    ComPtr<ID3D12Resource> persistent_resource)
    : persistent_buffer(std::move(persistent_resource)) {
  CHECK_GT(persistent_buffer_byte_length, 0u);
  CHECK_NE(persistent_buffer.Get(), nullptr);
  persistent_buffer_binding =
      DML_BUFFER_BINDING{.Buffer = persistent_buffer.Get(),
                         .Offset = 0,
                         .SizeInBytes = persistent_buffer_byte_length};
  persistent_buffer_binding_desc = DML_BINDING_DESC{
      .Type = DML_BINDING_TYPE_BUFFER, .Desc = &persistent_buffer_binding};
}

GraphImpl::PersistentResource::~PersistentResource() = default;

GraphImpl::ComputeResources::ComputeResources(
    ComPtr<ID3D12DescriptorHeap> descriptor_heap,
    AlignedByteLength<std::string> input_aligned_byte_length,
    ComPtr<ID3D12Resource> upload_buffer,
    ComPtr<ID3D12Resource> input_buffer,
    AlignedByteLength<std::string> output_aligned_byte_length,
    ComPtr<ID3D12Resource> output_buffer,
    ComPtr<ID3D12Resource> readback_buffer,
    uint64_t temporary_buffer_byte_length,
    ComPtr<ID3D12Resource> temporary_resource)
    : descriptor_heap(std::move(descriptor_heap)),
      input_aligned_byte_length(std::move(input_aligned_byte_length)),
      upload_buffer(std::move(upload_buffer)),
      input_buffer(std::move(input_buffer)),
      output_aligned_byte_length(std::move(output_aligned_byte_length)),
      output_buffer(std::move(output_buffer)),
      readback_buffer(std::move(readback_buffer)),
      temporary_buffer(std::move(temporary_resource)) {
  if (temporary_buffer_byte_length > 0) {
    CHECK_NE(temporary_buffer.Get(), nullptr);
    temporary_buffer_binding =
        DML_BUFFER_BINDING{.Buffer = temporary_buffer.Get(),
                           .Offset = 0,
                           .SizeInBytes = temporary_buffer_byte_length};
    temporary_buffer_binding_desc =
        DML_BINDING_DESC{.Type = DML_BINDING_TYPE_BUFFER,
                         .Desc = &temporary_buffer_binding.value()};
  }
}

GraphImpl::ComputeResources::~ComputeResources() = default;

// static
base::expected<std::unique_ptr<GraphImpl::ComputeResources>, HRESULT>
GraphImpl::AllocateComputeResources(
    CommandRecorder* command_recorder,
    IDMLCompiledOperator* compiled_operator,
    const ComputeResourceInfo& compute_resource_info) {
  TRACE_EVENT0("gpu", "GraphImpl::AllocateComputeResources");

  // Create the descriptor heap.
  DML_BINDING_PROPERTIES execution_binding_properties =
      compiled_operator->GetBindingProperties();
  ComPtr<ID3D12DescriptorHeap> descriptor_heap;
  RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateDescriptorHeap(
      execution_binding_properties.RequiredDescriptorCount,
      L"WebNN_Descriptor_Heap_For_Execution", descriptor_heap));

  // Calculate the total byte length of input array buffers to create
  // GPU input buffer and upload buffer, also records the aligned D3D12_RANGE
  // for each input.
  std::optional<AlignedByteLength<std::string>> aligned_byte_length_of_inputs =
      CalculateAlignedByteLength(
          compute_resource_info.input_name_to_byte_length_map);
  if (!aligned_byte_length_of_inputs) {
    DLOG(ERROR) << "Failed to calculate the aligned byte length of inputs.";
    return base::unexpected(E_INVALIDARG);
  }

  size_t total_byte_length_of_inputs =
      aligned_byte_length_of_inputs.value().total_byte_length;
  ComPtr<ID3D12Resource> upload_buffer;
  ComPtr<ID3D12Resource> input_buffer;
  // It is possible that a graph doesn't have any inputs. For example, a graph
  // may only compute results given weights. For such graphs, there is no need
  // to allocate upload and input buffers.
  if (total_byte_length_of_inputs > 0) {
    if (command_recorder->IsUMA()) {
      // For GPU supports UMA, create the custom heap with CPU memory pool, and
      // create a resource to map the heap. CPU writes the input data into this
      // resource which could be bound as graph input for GPU reading during
      // execution.
      RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateCustomUploadBuffer(
          total_byte_length_of_inputs, L"WebNN_Custom_Upload_Buffer_Inputs",
          input_buffer));
    } else {
      // Create the upload heap that can be written by CPU and read from GPU,
      // and create a resource to map the heap.
      RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateUploadBuffer(
          total_byte_length_of_inputs, L"WebNN_Upload_Buffer_Inputs",
          upload_buffer));
      // Create the default heap that only can be accessed by GPU not provide
      // CPU access, and create a resource to map the heap.
      RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateDefaultBuffer(
          total_byte_length_of_inputs, L"WebNN_Default_Buffer_Inputs",
          input_buffer));
    }
  }

  // Calculate the total byte length of outputs array buffer to create
  // an output buffer and readback buffer, also records the aligned D3D12_RANGE
  // for each output.
  std::optional<AlignedByteLength<std::string>> aligned_byte_length_of_outputs =
      CalculateAlignedByteLength(
          compute_resource_info.output_name_to_byte_length_map);
  if (!aligned_byte_length_of_outputs) {
    DLOG(ERROR) << "Failed to calculate the aligned byte length of outputs.";
    return base::unexpected(E_INVALIDARG);
  }

  // Create the output buffer which will be bound for the graph execution.
  size_t total_byte_length_of_outputs =
      aligned_byte_length_of_outputs.value().total_byte_length;
  ComPtr<ID3D12Resource> readback_buffer;
  ComPtr<ID3D12Resource> output_buffer;
  if (command_recorder->IsUMA()) {
    // For GPU supports UMA, create the custom heap with CPU memory pool, and
    // create a resource to map the heap. This resource could be bound as graph
    // execution output for GPU writing. And CPU could read the output data from
    // this resource after GPU execution.
    RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateCustomReadbackBuffer(
        total_byte_length_of_outputs, L"WebNN_Custom_Readback_Buffer_Outputs",
        output_buffer));
  } else {
    // Create the output buffer which will be written by GPU.
    RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateDefaultBuffer(
        total_byte_length_of_outputs, L"WebNN_Default_Buffer_Outputs",
        output_buffer));

    // Create the readback buffer which will be read by CPU.
    RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateReadbackBuffer(
        total_byte_length_of_outputs, L"WebNN_ReadBack_Buffer_Outputs",
        readback_buffer));
  }

  // Create and bind the temporary resource if the operator execution requires.
  ComPtr<ID3D12Resource> temporary_buffer;
  uint64_t temporary_buffer_byte_length =
      execution_binding_properties.TemporaryResourceSize;
  if (temporary_buffer_byte_length > 0) {
    RETURN_UNEXPECTED_IF_FAILED(command_recorder->CreateDefaultBuffer(
        temporary_buffer_byte_length, L"WebNN_Temporary_Buffer_For_Execution",
        temporary_buffer));
  }

  return base::WrapUnique(new ComputeResources(
      std::move(descriptor_heap),
      std::move(aligned_byte_length_of_inputs.value()),
      std::move(upload_buffer), std::move(input_buffer),
      std::move(aligned_byte_length_of_outputs.value()),
      std::move(output_buffer), std::move(readback_buffer),
      temporary_buffer_byte_length, std::move(temporary_buffer)));
}

// static
HRESULT GraphImpl::RecordGraphExecution(
    IDMLCompiledOperator* compiled_operator,
    CommandRecorder* command_recorder,
    const ComputeResources* compute_resources,
    const PersistentResource* persistent_resource,
    const GraphBufferBindingInfo& graph_buffer_binding_info) {
  // Open the command recorder for recording the graph execution commands.
  RETURN_IF_FAILED(command_recorder->Open());

  // Create the input buffer bindings for the graph execution.
  std::map<std::string, DML_BUFFER_BINDING>
      graph_input_name_to_buffer_binding_map;
  for (auto& [name, d3d12_range] :
       compute_resources->input_aligned_byte_length.key_to_d3d12_range_map) {
    auto size_in_bytes = d3d12_range.End - d3d12_range.Begin;
    graph_input_name_to_buffer_binding_map[name] =
        DML_BUFFER_BINDING{.Buffer = compute_resources->input_buffer.Get(),
                           .Offset = d3d12_range.Begin,
                           .SizeInBytes = size_in_bytes};
  }

  std::vector<DML_BINDING_DESC> input_buffer_binding_desc(
      graph_buffer_binding_info.input_buffer_binding_count,
      DML_BINDING_DESC{.Type = DML_BINDING_TYPE_NONE, .Desc = nullptr});

  // The graph input tensors must be bound to the binding table during the
  // graph execution.
  for (auto& [name, buffer_binding] : graph_input_name_to_buffer_binding_map) {
    // Get the graph input index with the name.
    const auto graph_input_index_iterator =
        graph_buffer_binding_info.graph_input_name_to_index_map.find(name);
    CHECK(graph_input_index_iterator !=
          graph_buffer_binding_info.graph_input_name_to_index_map.end());
    uint32_t graph_input_index = graph_input_index_iterator->second;
    input_buffer_binding_desc[graph_input_index] = {DML_BINDING_TYPE_BUFFER,
                                                    &buffer_binding};
  }

  if (compute_resources->input_aligned_byte_length.total_byte_length > 0 &&
      !command_recorder->IsUMA()) {
    UploadBufferWithBarrier(
        command_recorder, compute_resources->input_buffer,
        compute_resources->upload_buffer,
        compute_resources->input_aligned_byte_length.total_byte_length);
  }

  // Create the output buffer bindings for the graph execution.
  size_t output_buffer_binding_count =
      graph_buffer_binding_info.graph_output_name_to_index_map.size();
  std::vector<DML_BINDING_DESC> output_buffer_binding_desc(
      output_buffer_binding_count,
      DML_BINDING_DESC{.Type = DML_BINDING_TYPE_NONE, .Desc = nullptr});
  std::vector<DML_BUFFER_BINDING> output_buffer_binding;
  output_buffer_binding.reserve(output_buffer_binding_count);

  for (auto& [name, graph_output_index] :
       graph_buffer_binding_info.graph_output_name_to_index_map) {
    const auto graph_output_range_iterator =
        compute_resources->output_aligned_byte_length.key_to_d3d12_range_map
            .find(name);
    CHECK(graph_output_range_iterator !=
          compute_resources->output_aligned_byte_length.key_to_d3d12_range_map
              .end());
    const auto& d3d12_range = graph_output_range_iterator->second;
    output_buffer_binding.push_back(
        DML_BUFFER_BINDING{.Buffer = compute_resources->output_buffer.Get(),
                           .Offset = d3d12_range.Begin,
                           .SizeInBytes = d3d12_range.End - d3d12_range.Begin});
    output_buffer_binding_desc[graph_output_index] = {
        DML_BINDING_TYPE_BUFFER, &output_buffer_binding.back()};
  }

  std::optional<DML_BINDING_DESC> persistent_buffer_binding_desc;
  if (persistent_resource) {
    persistent_buffer_binding_desc =
        persistent_resource->persistent_buffer_binding_desc;
  }

  // Execute the graph with input, output and persistent buffer bindings.
  RETURN_IF_FAILED(command_recorder->ExecuteOperator(
      compiled_operator, compute_resources->descriptor_heap,
      input_buffer_binding_desc, output_buffer_binding_desc,
      persistent_buffer_binding_desc,
      compute_resources->temporary_buffer_binding_desc));

  if (!command_recorder->IsUMA()) {
    ReadbackBufferWithBarrier(
        command_recorder, compute_resources->readback_buffer,
        compute_resources->output_buffer,
        compute_resources->output_aligned_byte_length.total_byte_length);
  }

  RETURN_IF_FAILED(command_recorder->Close());
  return S_OK;
}

GraphImpl::GraphImpl(std::unique_ptr<CommandRecorder> command_recorder,
                     std::unique_ptr<PersistentResource> persistent_resource,
                     ComPtr<IDMLCompiledOperator> compiled_operator,
                     ComputeResourceInfo compute_resource_info,
                     GraphBufferBindingInfo graph_buffer_binding_info,
                     std::unique_ptr<ComputeResources> compute_resources)
    : WebNNGraphImpl(std::move(compute_resource_info)),
      persistent_resource_(std::move(persistent_resource)),
      command_recorder_(std::move(command_recorder)),
      compiled_operator_(std::move(compiled_operator)),
      graph_buffer_binding_info_(std::move(graph_buffer_binding_info)),
      compute_resources_(std::move(compute_resources)) {
  command_queue_ = command_recorder_->GetCommandQueue();
  dml_device_ = command_recorder_->GetDMLDevice();
}

//  Notice that it's the CommandQueue's responsibility to wait for all of the
//  queued work to complete before destructing itself.
GraphImpl::~GraphImpl() = default;

ComPtr<IDMLCompiledOperator> GraphImpl::CompileOnBackgroundThread(
    GraphBuilder graph_builder) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::CompileOnBackgroundThread");
  return graph_builder.Compile(DML_EXECUTION_FLAG_NONE);
}

// static
void GraphImpl::OnCompilationComplete(
    mojom::WebNNContext::CreateGraphCallback callback,
    std::unique_ptr<CommandRecorder> command_recorder,
    base::flat_map<uint64_t, mojo_base::BigBuffer> constant_id_to_buffer_map,
    std::unordered_map<uint64_t, uint32_t> constant_id_to_input_index_map,
    GraphBufferBindingInfo graph_buffer_binding_info,
    ComputeResourceInfo compute_resource_info,
    ComPtr<IDMLCompiledOperator> compiled_operator) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::OnCompilationComplete");
  if (!compiled_operator) {
    HandleGraphCreationFailure("Failed to compile the graph.",
                               std::move(callback));
    return;
  }

  HRESULT hr = command_recorder->Open();
  if (FAILED(hr)) {
    HandleGraphCreationFailure("Failed to open the command recorder.", hr,
                               std::move(callback));
    return;
  }

  // Create the input resource binding for graph initialization. The number of
  // bindings must exactly match the number of inputs (including constants) of
  // the graph, only the constant resource needs to be bound, the inputs for
  // computation supply nullptr for `Buffer` member to indicate 'no binding'.
  //
  // The constant tensor specifying DML_TENSOR_FLAG_OWNED_BY_DML need to bind
  // the resource in the buffer binding (DML_BUFFER_BINDING) array, the index
  // of constant in the array is DML_INPUT_GRAPH_EDGE_DESC.GraphInputIndex which
  // is got from `constant_id_to_input_index_map`.
  //
  // The inputs tensors without the DML_TENSOR_FLAG_OWNED_BY_DML flag is
  // expected to be bound during execution, and not during initialization.
  std::vector<DML_BUFFER_BINDING> input_buffer_binding(
      graph_buffer_binding_info.input_buffer_binding_count,
      DML_BUFFER_BINDING{.Buffer = nullptr, .Offset = 0, .SizeInBytes = 0});
  if (!constant_id_to_buffer_map.empty()) {
    std::map<uint64_t, size_t> constant_id_to_byte_length_map;
    for (auto& [key, buffer] : constant_id_to_buffer_map) {
      constant_id_to_byte_length_map[key] = buffer.size();
    }

    std::optional<AlignedByteLength<uint64_t>>
        aligned_byte_length_of_constants =
            CalculateAlignedByteLength(constant_id_to_byte_length_map);
    if (!aligned_byte_length_of_constants) {
      HandleGraphCreationFailure(
          "Failed to calculate the aligned byte length of constants.",
          std::move(callback));
      return;
    }

    size_t total_byte_length_of_constants =
        aligned_byte_length_of_constants.value().total_byte_length;
    absl::variant<UploadAndDefaultBuffers, ComPtr<ID3D12Resource>>
        buffer_variant;
    if (command_recorder->IsUMA()) {
      // For GPU supports UMA, create the custom heap with CPU memory pool, and
      // create a resource to map the heap. CPU writes constants into this
      // resource which will be bound as graph input for GPU reading during
      // initialization.
      ComPtr<ID3D12Resource> cpu_buffer;
      hr = command_recorder->CreateCustomUploadBuffer(
          total_byte_length_of_constants,
          L"WebNN_Custom_Upload_Buffer_Constants", cpu_buffer);
      if (FAILED(hr)) {
        HandleGraphCreationFailure(
            "Failed to create custom upload buffer for constants.", hr,
            std::move(callback));
        return;
      }
      buffer_variant = std::move(cpu_buffer);
    } else {
      // Create the upload heap that can be written by CPU and read from GPU,
      // and create a resource to map the heap.
      ComPtr<ID3D12Resource> upload_buffer;
      hr = command_recorder->CreateUploadBuffer(
          total_byte_length_of_constants, L"WebNN_Upload_Buffer_Constants",
          upload_buffer);
      if (FAILED(hr)) {
        HandleGraphCreationFailure(
            "Failed to create upload buffer for constants.", hr,
            std::move(callback));
        return;
      }
      // Create the default heap that only can be accessed by GPU not provide
      // CPU access, and create a resource to map the heap.
      ComPtr<ID3D12Resource> default_buffer;
      hr = command_recorder->CreateDefaultBuffer(
          total_byte_length_of_constants, L"WebNN_Default_Buffer_Constants",
          default_buffer);
      if (FAILED(hr)) {
        HandleGraphCreationFailure(
            "Failed to create default input buffer for constants.", hr,
            std::move(callback));
        return;
      }
      buffer_variant =
          UploadAndDefaultBuffers{.upload_buffer = std::move(upload_buffer),
                                  .default_buffer = std::move(default_buffer)};
    }

    auto constant_buffer_binding = UploadAndCreateConstantBufferBinding(
        command_recorder.get(), constant_id_to_buffer_map,
        aligned_byte_length_of_constants.value(), std::move(buffer_variant));
    if (!constant_buffer_binding) {
      HandleGraphCreationFailure("Failed to upload constant weight data.",
                                 std::move(callback));
      return;
    }
    // The constant tensor must be bound to the binding table during operator
    // initialization, and not during execution.
    for (auto& [constant_id, buffer_binding] :
         constant_buffer_binding.value()) {
      // Get the graph input index with the constant id.
      const auto graph_input_index_iterator =
          constant_id_to_input_index_map.find(constant_id);
      CHECK(graph_input_index_iterator != constant_id_to_input_index_map.end());
      input_buffer_binding[graph_input_index_iterator->second] =
          std::move(buffer_binding);
    }
  }
  DML_BUFFER_ARRAY_BINDING input_buffer_array_binding{
      .BindingCount = base::checked_cast<uint32_t>(input_buffer_binding.size()),
      .Bindings = input_buffer_binding.data()};
  DML_BINDING_DESC input_buffer_binding_desc = {DML_BINDING_TYPE_BUFFER_ARRAY,
                                                &input_buffer_array_binding};

  // Create the persistent resource which is bound as output of operator
  // initializer.
  std::unique_ptr<PersistentResource> persistent_resource;
  std::optional<DML_BINDING_DESC> persistent_buffer_binding_desc;
  DML_BINDING_PROPERTIES execution_binding_properties =
      compiled_operator->GetBindingProperties();
  uint64_t persistent_buffer_size =
      execution_binding_properties.PersistentResourceSize;
  if (persistent_buffer_size) {
    ComPtr<ID3D12Resource> persistent_buffer;
    hr = command_recorder->CreateDefaultBuffer(
        persistent_buffer_size, L"WebNN_Default_Persistent_Buffer",
        persistent_buffer);
    if (FAILED(hr)) {
      HandleGraphCreationFailure(
          "Failed to create the default buffer for persistent resource.", hr,
          std::move(callback));
      return;
    }

    persistent_resource = base::WrapUnique(new PersistentResource(
        persistent_buffer_size, std::move(persistent_buffer)));
    persistent_buffer_binding_desc =
        persistent_resource->persistent_buffer_binding_desc;
  }

  hr = command_recorder->InitializeOperator(compiled_operator.Get(),
                                            input_buffer_binding_desc,
                                            persistent_buffer_binding_desc);
  if (FAILED(hr)) {
    HandleGraphCreationFailure("Failed to initialize the operator.", hr,
                               std::move(callback));
    return;
  }

  hr = command_recorder->CloseAndExecute();
  if (FAILED(hr)) {
    HandleGraphCreationFailure("Failed to close and execute the command list.",
                               hr, std::move(callback));
    return;
  }

  scoped_refptr<CommandQueue> command_queue(
      command_recorder->GetCommandQueue());

  command_queue->WaitAsync(base::BindOnce(
      &GraphImpl::OnInitializationComplete, std::move(command_recorder),
      std::move(persistent_resource), std::move(compiled_operator),
      std::move(compute_resource_info), std::move(graph_buffer_binding_info),
      std::move(callback)));
}

// static
void GraphImpl::OnInitializationComplete(
    std::unique_ptr<CommandRecorder> command_recorder,
    std::unique_ptr<PersistentResource> persistent_resource,
    ComPtr<IDMLCompiledOperator> compiled_operator,
    ComputeResourceInfo compute_resource_info,
    GraphBufferBindingInfo graph_buffer_binding_info,
    mojom::WebNNContext::CreateGraphCallback callback,
    HRESULT hr) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::OnInitializationComplete");
  if (FAILED(hr)) {
    HandleGraphCreationFailure(
        "Failed to wait for the initialization to complete.", hr,
        std::move(callback));
    return;
  }

  // Release the resources used for graph initialization.
  command_recorder->GetCommandQueue()->ReleaseCompletedResources();

  base::expected<std::unique_ptr<ComputeResources>, HRESULT>
      compute_resources_allocation_result = AllocateComputeResources(
          command_recorder.get(), compiled_operator.Get(),
          compute_resource_info);
  if (!compute_resources_allocation_result.has_value()) {
    HandleGraphCreationFailure(
        "Failed to allocate compute resource.",
        std::move(compute_resources_allocation_result.error()),
        std::move(callback));
    return;
  }
  std::unique_ptr<ComputeResources> compute_resources =
      std::move(compute_resources_allocation_result.value());
  CHECK(compute_resources);

  hr = RecordGraphExecution(compiled_operator.Get(), command_recorder.get(),
                            compute_resources.get(), persistent_resource.get(),
                            graph_buffer_binding_info);
  if (FAILED(hr)) {
    HandleGraphCreationFailure(
        "Failed to record commands and bind resources for execution.", hr,
        std::move(callback));
    return;
  }

  scoped_refptr<CommandQueue> command_queue(
      command_recorder->GetCommandQueue());
  // The remote sent to the renderer.
  mojo::PendingRemote<mojom::WebNNGraph> blink_remote;
  // The receiver bound to GraphImpl.
  mojo::MakeSelfOwnedReceiver<mojom::WebNNGraph>(
      base::WrapUnique(new GraphImpl(
          std::move(command_recorder), std::move(persistent_resource),
          std::move(compiled_operator), std::move(compute_resource_info),
          std::move(graph_buffer_binding_info), std::move(compute_resources))),
      blink_remote.InitWithNewPipeAndPassReceiver());
  command_queue->ReleaseCompletedResources();
  std::move(callback).Run(
      CreateGraphResult::NewGraphRemote(std::move(blink_remote)));
}

// static
void GraphImpl::CreateAndBuild(
    scoped_refptr<CommandQueue> command_queue,
    ComPtr<IDMLDevice> dml_device,
    mojom::GraphInfoPtr graph_info,
    mojom::WebNNContext::CreateGraphCallback callback) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::CreateAndBuild");
  // `CommandRecorder` would keep reference of command queue and DML device.
  std::unique_ptr<CommandRecorder> command_recorder =
      CommandRecorder::Create(command_queue, dml_device);
  if (!command_recorder) {
    HandleGraphCreationFailure("Failed to open the command recorder.",
                               std::move(callback));
    return;
  }

  GraphBuilder graph_builder(dml_device);
  IdToNodeOutputMap id_to_node_output_map;
  const IdToOperandMap& id_to_operand_map = graph_info->id_to_operand_map;
  std::unordered_map<uint64_t, uint32_t> constant_id_to_input_index_map;
  GraphBufferBindingInfo graph_buffer_binding_info;
  // Add inputs.
  for (auto& input_id : graph_info->input_operands) {
    auto graph_input_index = CreateInputNode(
        id_to_operand_map, input_id, graph_builder, id_to_node_output_map);
    const OperandPtr& operand = id_to_operand_map.at(input_id);
    CHECK(operand);
    graph_buffer_binding_info
        .graph_input_name_to_index_map[operand->name.value()] =
        graph_input_index;
  }

  // The constant operand in WebNNGraph also is treated as input node in graph
  // desc.
  for (auto& [constant_id, _] : graph_info->constant_id_to_buffer_map) {
    auto graph_input_index = CreateInputNode(
        id_to_operand_map, constant_id, graph_builder, id_to_node_output_map);
    constant_id_to_input_index_map[constant_id] = graph_input_index;
  }

  // Find out the next operand id that can be used as the key in
  // `id_to_operand_map`. It might be used for inserting new operands into maps
  // when adding operations.
  uint64_t next_operand_id = 0;
  base::ranges::for_each(
      id_to_operand_map, [&next_operand_id](auto& key_value) {
        next_operand_id = std::max(next_operand_id, key_value.first + 1);
      });

  // Add operations.
  for (auto& operation : graph_info->operations) {
    // For operators that deal with DML API, there is a chance that operator
    // creation will fail. Use `mojom::ErrorPtr` to hold the given error
    // message.
    base::expected<void, mojom::ErrorPtr> create_operator_result;
    switch (operation->which()) {
      case Operation::Tag::kArgMinMax: {
        create_operator_result = CreateOperatorNodeForArgMinMax(
            id_to_operand_map, operation->get_arg_min_max(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kBatchNormalization: {
        create_operator_result = CreateOperatorNodeForBatchNormalization(
            operation->get_batch_normalization(), graph_info, graph_builder,
            id_to_node_output_map, constant_id_to_input_index_map,
            next_operand_id);
        break;
      }
      case Operation::Tag::kClamp: {
        create_operator_result = CreateOperatorNodeForClamp(
            id_to_operand_map, operation->get_clamp(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kConcat: {
        create_operator_result = CreateOperatorNodeForConcat(
            id_to_operand_map, operation->get_concat(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kConv2d: {
        create_operator_result = CreateOperatorNodeForConv2d(
            id_to_operand_map, operation->get_conv2d(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kElementWiseBinary: {
        create_operator_result = CreateOperatorNodeForBinary(
            id_to_operand_map, operation->get_element_wise_binary(),
            graph_builder, id_to_node_output_map);
        break;
      }
      case Operation::Tag::kElu: {
        create_operator_result =
            CreateOperatorNodeForElu(id_to_operand_map, operation->get_elu(),
                                     graph_builder, id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kElementWiseUnary: {
        create_operator_result = CreateOperatorNodeForElementWiseUnary(
            id_to_operand_map, operation->get_element_wise_unary(),
            graph_builder, id_to_node_output_map);
        break;
      }
      case Operation::Tag::kExpand: {
        create_operator_result = CreateOperatorNodeForExpand(
            id_to_operand_map, operation->get_expand(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kGather: {
        create_operator_result = CreateOperatorNodeForGather(
            id_to_operand_map, operation->get_gather(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kGemm: {
        create_operator_result =
            CreateOperatorNodeForGemm(id_to_operand_map, operation->get_gemm(),
                                      graph_builder, id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kHardSigmoid: {
        create_operator_result = CreateOperatorNodeForHardSigmoid(
            id_to_operand_map, operation->get_hard_sigmoid(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kHardSwish: {
        create_operator_result = CreateOperatorNodeForHardSwish(
            id_to_operand_map, operation->get_hard_swish(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kInstanceNormalization: {
        // The axes along which to calculate the Mean and Variance.
        std::array<uint32_t, 2> mean_variance_axes;
        std::array<uint32_t, 1> scale_bias_broadcast_axes;
        const auto& instance_normalization =
            operation->get_instance_normalization();
        switch (instance_normalization->layout) {
          case mojom::InputOperandLayout::kChannelsFirst: {
            mean_variance_axes = {2, 3};
            scale_bias_broadcast_axes = {1};
            break;
          }
          case mojom::InputOperandLayout::kChannelsLast:
            mean_variance_axes = {1, 2};
            scale_bias_broadcast_axes = {3};
            break;
        }
        create_operator_result = CreateOperatorNodeForMeanVarianceNormalization(
            instance_normalization, graph_info, graph_builder,
            id_to_node_output_map, constant_id_to_input_index_map,
            next_operand_id, mean_variance_axes, scale_bias_broadcast_axes,
            Operation::Tag::kInstanceNormalization);
        break;
      }
      case Operation::Tag::kLayerNormalization: {
        const auto& layer_normalization = operation->get_layer_normalization();
        const auto axes = layer_normalization->axes;
        create_operator_result = CreateOperatorNodeForMeanVarianceNormalization(
            layer_normalization, graph_info, graph_builder,
            id_to_node_output_map, constant_id_to_input_index_map,
            next_operand_id, axes, axes, Operation::Tag::kLayerNormalization);
        break;
      }
      case Operation::Tag::kLeakyRelu: {
        create_operator_result = CreateOperatorNodeForLeakyRelu(
            id_to_operand_map, operation->get_leaky_relu(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kLinear: {
        create_operator_result = CreateOperatorNodeForLinear(
            id_to_operand_map, operation->get_linear(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kMatmul: {
        create_operator_result = CreateOperatorNodeForMatmul(
            id_to_operand_map, operation->get_matmul(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kPad: {
        create_operator_result =
            CreateOperatorNodeForPad(id_to_operand_map, operation->get_pad(),
                                     graph_builder, id_to_node_output_map);
        break;
      }
      case Operation::Tag::kPool2d: {
        create_operator_result = CreateOperatorNodeForPool2d(
            id_to_operand_map, operation->get_pool2d(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kPrelu: {
        create_operator_result = CreateOperatorNodeForPrelu(
            id_to_operand_map, operation->get_prelu(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kReduce: {
        create_operator_result = CreateOperatorNodeForReduce(
            id_to_operand_map, operation->get_reduce(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kRelu: {
        create_operator_result =
            CreateOperatorNodeForUnary<DML_ACTIVATION_RELU_OPERATOR_DESC,
                                       DML_OPERATOR_ACTIVATION_RELU>(
                id_to_operand_map, operation->get_relu(), graph_builder,
                id_to_node_output_map);
        break;
      }
      case Operation::Tag::kResample2d: {
        create_operator_result = CreateOperatorNodeForResample2d(
            id_to_operand_map, operation->get_resample2d(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kReshape: {
        CreateNodeOutputForReshape(id_to_operand_map, operation->get_reshape(),
                                   graph_builder, id_to_node_output_map);
        break;
      }
      case Operation::Tag::kSigmoid: {
        create_operator_result =
            CreateOperatorNodeForUnary<DML_ACTIVATION_SIGMOID_OPERATOR_DESC,
                                       DML_OPERATOR_ACTIVATION_SIGMOID>(
                id_to_operand_map, operation->get_sigmoid(), graph_builder,
                id_to_node_output_map);

        break;
      }
      case Operation::Tag::kSlice: {
        create_operator_result = CreateOperatorNodeForSlice(
            id_to_operand_map, operation->get_slice(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kSoftmax: {
        create_operator_result =
            CreateOperatorNodeForUnary<DML_ACTIVATION_SOFTMAX_OPERATOR_DESC,
                                       DML_OPERATOR_ACTIVATION_SOFTMAX>(
                id_to_operand_map, operation->get_softmax(), graph_builder,
                id_to_node_output_map);
        break;
      }
      case mojom::Operation::Tag::kSoftplus: {
        create_operator_result = CreateOperatorNodeForSoftplus(
            id_to_operand_map, operation->get_softplus(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kSoftsign: {
        create_operator_result =
            CreateOperatorNodeForUnary<DML_ACTIVATION_SOFTSIGN_OPERATOR_DESC,
                                       DML_OPERATOR_ACTIVATION_SOFTSIGN>(
                id_to_operand_map, operation->get_softsign(), graph_builder,
                id_to_node_output_map);

        break;
      }
      case mojom::Operation::Tag::kSplit: {
        create_operator_result = CreateOperatorNodeForSplit(
            id_to_operand_map, operation->get_split(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kTanh: {
        create_operator_result =
            CreateOperatorNodeForUnary<DML_ACTIVATION_TANH_OPERATOR_DESC,
                                       DML_OPERATOR_ACTIVATION_TANH>(
                id_to_operand_map, operation->get_tanh(), graph_builder,
                id_to_node_output_map);
        break;
      }
      case Operation::Tag::kTranspose: {
        create_operator_result = CreateOperatorNodeForTranspose(
            id_to_operand_map, operation->get_transpose(), graph_builder,
            id_to_node_output_map);
        break;
      }
      case Operation::Tag::kWhere: {
        create_operator_result = CreateOperatorNodeForWhere(
            id_to_operand_map, operation->get_where(), graph_builder,
            id_to_node_output_map);
        break;
      }
      default: {
        std::string error_message = "This operator (" +
                                    OpTagToString(operation->which()) +
                                    ") is not supported.";
        DLOG(ERROR) << error_message;
        create_operator_result = base::unexpected(CreateError(
            mojom::Error::Code::kNotSupportedError, std::move(error_message)));
      }
    }
    if (!create_operator_result.has_value()) {
      std::move(callback).Run(CreateGraphResult::NewError(
          std::move(create_operator_result.error())));
      return;
    }
  }

  for (auto& output_id : graph_info->output_operands) {
    const auto output_iterator = id_to_node_output_map.find(output_id);
    CHECK(output_iterator != id_to_node_output_map.end());
    const NodeOutput* output = output_iterator->second;
    CHECK(output);
    // TODO: A DML graph's output tensor may have adjusted strides rather than
    // default strides which are calculated by its' dimensions. For example,
    // dimensions [1,2,3,4] should have default strides [24,12,4,1] according to
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/dml-helper-functions#calculatestrides,
    // but the strides may be adjusted for supporting some ops such as
    // transpose. Append an identity operator to consume the adjusted strides to
    // ensure a correct output result.

    // Appending an identity operator DML_OPERATOR_ELEMENT_WISE_IDENTITY which
    // effectively copies input tensor to the output tensor to avoid directly
    // using graph input as output.
    const TensorDesc& output_tensor_desc = output->GetTensorDesc();
    auto output_type = output->GetNode().GetType();
    if (output_type == Node::Type::kInput) {
      TensorDesc identity_tensor_desc(output_tensor_desc.GetDataType(),
                                      DML_TENSOR_FLAG_NONE,
                                      output_tensor_desc.GetDimensions());
      const OperatorNode* identity_node =
          CreateUnaryOperator<DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC,
                              DML_OPERATOR_ELEMENT_WISE_IDENTITY>(
              output_tensor_desc, identity_tensor_desc, output, graph_builder);
      if (!identity_node) {
        HandleGraphCreationFailure("Failed to create identity operator.",
                                   std::move(callback));
        return;
      }

      output = graph_builder.CreateNodeOutput(identity_node,
                                              std::move(identity_tensor_desc));
    }

    std::string name = id_to_operand_map.at(output_id)->name.value();
    graph_buffer_binding_info.graph_output_name_to_index_map[std::move(name)] =
        graph_builder.CreateOutputEdge(output);
  }

  graph_buffer_binding_info.input_buffer_binding_count =
      constant_id_to_input_index_map.size() +
      graph_buffer_binding_info.graph_input_name_to_index_map.size();

  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE,
      {base::TaskPriority::USER_BLOCKING,
       base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::BindOnce(&GraphImpl::CompileOnBackgroundThread,
                     std::move(graph_builder)),
      base::BindOnce(&GraphImpl::OnCompilationComplete, std::move(callback),
                     std::move(command_recorder),
                     std::move(graph_info->constant_id_to_buffer_map),
                     std::move(constant_id_to_input_index_map),
                     std::move(graph_buffer_binding_info),
                     ComputeResourceInfo(graph_info)));
}

void GraphImpl::HandleComputationFailure(
    const std::string& error_message,
    mojom::WebNNGraph::ComputeCallback callback) {
  DLOG(ERROR) << error_message;
  command_recorder_.reset();
  std::move(callback).Run(ComputeResult::NewError(
      CreateError(mojom::Error::Code::kUnknownError, error_message)));
}

void GraphImpl::HandleComputationFailure(
    const std::string& error_message,
    HRESULT hr,
    mojom::WebNNGraph::ComputeCallback callback) {
  DLOG(ERROR) << error_message << " " << logging::SystemErrorCodeToString(hr);
  command_recorder_.reset();
  if (hr == E_OUTOFMEMORY) {
    DLOG(ERROR) << "No enough memory resources are available.";
    std::move(callback).Run(ComputeResult::NewError(CreateError(
        mojom::Error::Code::kUnknownError,
        error_message + " No enough memory resources are available.")));
  } else {
    std::move(callback).Run(ComputeResult::NewError(
        CreateError(mojom::Error::Code::kUnknownError, error_message)));
  }
}

void GraphImpl::ComputeImpl(
    base::flat_map<std::string, mojo_base::BigBuffer> named_inputs,
    mojom::WebNNGraph::ComputeCallback callback) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::ComputeImpl");

  // It indicates whether we need to record commands and bind resources again
  // for the graph execution by calling `RecordGraphExecution` method. If either
  // the `compute_resources_` or `command_recorder_` is not available during the
  // graph execution, it must be set to true.
  bool is_command_recording_needed = false;

  // Recreate the command recorder if it has been released by last failed
  // computation or it is unavailable due to still being occupied by last
  // computation.
  if (!command_recorder_) {
    command_recorder_ = CommandRecorder::Create(command_queue_, dml_device_);
    if (!command_recorder_) {
      HandleComputationFailure("Failed to create the command recorder.",
                               std::move(callback));
      return;
    }
    is_command_recording_needed = true;
  }

  std::unique_ptr<CommandRecorder> command_recorder =
      std::move(command_recorder_);

  // Use the existing compute resource if it is available, otherwise allocate
  // a new one.
  std::unique_ptr<ComputeResources> compute_resources =
      std::move(compute_resources_);
  if (!compute_resources) {
    base::expected<std::unique_ptr<ComputeResources>, HRESULT>
        compute_resources_allocation_result = AllocateComputeResources(
            command_recorder.get(), compiled_operator_.Get(),
            compute_resource_info());
    if (!compute_resources_allocation_result.has_value()) {
      HandleComputationFailure(
          "Failed to allocate compute resource.",
          std::move(compute_resources_allocation_result.error()),
          std::move(callback));
      return;
    }
    compute_resources = std::move(compute_resources_allocation_result.value());
    is_command_recording_needed = true;
  }
  CHECK(compute_resources);

  HRESULT hr = S_OK;

  if (is_command_recording_needed) {
    hr = RecordGraphExecution(compiled_operator_.Get(), command_recorder.get(),
                              compute_resources.get(),
                              persistent_resource_.get(),
                              graph_buffer_binding_info_);
    if (FAILED(hr)) {
      HandleComputationFailure(
          "Failed to record and bind resources for execution.", hr,
          std::move(callback));
      return;
    }
  }

  if (compute_resources->input_aligned_byte_length.total_byte_length > 0) {
    // For GPU supports UMA, the `input_buffer` is allocated in the custom heap
    // which can be mapped and written by CPU efficiently.
    auto* buffer = command_recorder->IsUMA()
                       ? compute_resources->input_buffer.Get()
                       : compute_resources->upload_buffer.Get();
    hr = MapAndCopyInputDataToBuffer(
        named_inputs,
        compute_resources->input_aligned_byte_length.key_to_d3d12_range_map,
        buffer);
    if (FAILED(hr)) {
      HandleComputationFailure(
          "Failed to copy the data from named inputs to the buffer.", hr,
          std::move(callback));
      return;
    }
  }

  // Submit the command list for execution.
  hr = command_recorder->Execute();
  if (FAILED(hr)) {
    HandleComputationFailure("Failed to execute the command list.", hr,
                             std::move(callback));
    return;
  }

  command_queue_->WaitAsync(base::BindOnce(
      &GraphImpl::OnComputationComplete, weak_factory_.GetWeakPtr(),
      std::move(callback), std::move(compute_resources),
      std::move(command_recorder)));
}

void GraphImpl::OnComputationComplete(
    mojom::WebNNGraph::ComputeCallback callback,
    std::unique_ptr<ComputeResources> compute_resources,
    std::unique_ptr<CommandRecorder> command_recorder,
    HRESULT hr) {
  TRACE_EVENT0("gpu", "dml::GraphImpl::OnComputationComplete");
  if (FAILED(hr)) {
    HandleComputationFailure("Failed to wait for the computation to complete.",
                             hr, std::move(callback));
    return;
  }

  // Map entire buffer to readback the output data one by one with byte
  // offset. For GPU supports UMA, the `output_buffer` is allocated in the
  // custom heap that can be mapped and read by CPU efficiently.
  void* mapped_buffer = nullptr;
  auto* buffer_to_map = command_recorder->IsUMA()
                            ? compute_resources->output_buffer.Get()
                            : compute_resources->readback_buffer.Get();
  CHECK(buffer_to_map);
  hr = buffer_to_map->Map(0, nullptr, &mapped_buffer);
  if (FAILED(hr)) {
    HandleComputationFailure("Failed to map the buffer for outputs.", hr,
                             std::move(callback));
    return;
  }

  const std::map<std::string, D3D12_RANGE>&
      graph_output_name_to_d3d12_range_map =
          compute_resources->output_aligned_byte_length.key_to_d3d12_range_map;
  base::flat_map<std::string, mojo_base::BigBuffer> named_outputs;
  named_outputs.reserve(graph_output_name_to_d3d12_range_map.size());
  for (auto& [name, d3d12_range] : graph_output_name_to_d3d12_range_map) {
    named_outputs[name] = mojo_base::BigBuffer(base::make_span(
        static_cast<const uint8_t*>(mapped_buffer) + d3d12_range.Begin,
        compute_resource_info().output_name_to_byte_length_map.at(name)));
  }

  buffer_to_map->Unmap(0, nullptr);

  // If there is an existing available compute resource, release this compute
  // resource. Otherwise, recycle this compute resource for the next call.
  if (!compute_resources_) {
    compute_resources_ = std::move(compute_resources);
  }

  // Similarly, if there is an existing available command_recorder, release
  // it. Otherwise, recycle it for the next call.
  if (!command_recorder_) {
    command_recorder_ = std::move(command_recorder);
  }

  command_queue_->ReleaseCompletedResources();
  std::move(callback).Run(
      ComputeResult::NewNamedOutputs(std::move(named_outputs)));
}

}  // namespace webnn::dml
