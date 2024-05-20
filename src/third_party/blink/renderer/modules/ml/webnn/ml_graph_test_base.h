// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_TEST_BASE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_TEST_BASE_H_

#include <numeric>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_testing.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_compute_result.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/ml/buildflags.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_builder_test.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_builder_utils.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_operand.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"
#include "third_party/blink/renderer/platform/testing/task_environment.h"

namespace blink {

class MLGraphBuilder;
class V8TestingScope;

// The utility methods for graph test.
// The backends share the unit tests in the MLGraphTest.
enum class BackendType { kFake, kXnnpack, kWebNNService };

std::string TestParamInfoToString(
    const ::testing::TestParamInfo<BackendType>& backend_type);

std::pair<String, String> GetErrorNameAndMessage(V8TestingScope* scope,
                                                 ScriptValue value);

class MLGraphTestBase : public ::testing::Test,
                        public ::testing::WithParamInterface<BackendType> {
 public:
  // BuildResult is returned by Build() method. If the graph building is
  // successful, `graph` points to the MLGraph and `error_name` and
  // `error_message` are null. Otherwise, `graph` is a nullptr and
  // `error_name` and `error_message` are populated from the JS error or
  // DOMException.
  struct BuildResult {
    Persistent<MLGraph> graph;
    String error_name;
    String error_message;
  };

  // Helper method for testing BuildImpl() with the same named operands and
  // expected results.
  BuildResult BuildGraph(V8TestingScope& scope,
                         MLGraphBuilder* builder,
                         const MLNamedOperands& named_operands);

  // Helper method for testing Compute() with the same input/output buffers and
  // expected results. If the graph computes successfully, both Strings returned
  // are null and the results are produced into the output buffers. Otherwise,
  // it returns the name and message from the error thrown by the graph
  // computation.
  std::pair<String, String> ComputeGraph(V8TestingScope& scope,
                                         MLGraph* graph,
                                         MLNamedArrayBufferViews& inputs,
                                         MLNamedArrayBufferViews& outputs);

  // Helper method for testing both context and ML graph builder creation.
  // If the context cannot be created for the graph, returns nullptr.
  static MLGraphBuilder* CreateGraphBuilder(V8TestingScope& scope,
                                            MLContextOptions* options);

  // Helper method for testing only context creation.
  static ScriptPromiseUntyped CreateContext(
      V8TestingScope& scope,
      MLContextOptions* options = MLContextOptions::Create());

 private:
  test::TaskEnvironment task_environment_;
};

template <typename T>
struct OperandInfo {
  V8MLOperandDataType::Enum data_type;
  Vector<uint32_t> dimensions;
  Vector<T> values;
};

// Helper function to set the data of an ArrayBufferView from a vector.
template <typename T>
void SetArrayBufferViewValues(NotShared<DOMArrayBufferView> array_buffer_view,
                              const Vector<T>& values) {
  DCHECK_EQ(array_buffer_view->byteLength(), values.size() * sizeof(T));
  memcpy(array_buffer_view->BaseAddress(), values.data(),
         values.size() * sizeof(T));
}

// Overrode helper function to create an ArrayBufferView given an operand and
// set its data from a vector.
template <typename T>
NotShared<DOMArrayBufferView> CreateArrayBufferViewForOperand(
    const MLOperand* operand,
    const Vector<T>& values) {
  auto array_buffer_view = CreateArrayBufferViewForOperand(operand);
  SetArrayBufferViewValues(array_buffer_view, values);
  return array_buffer_view;
}

// Helper function to get the data of an ArrayBufferView into a vector.
template <typename T>
Vector<T> GetArrayBufferViewValues(
    NotShared<DOMArrayBufferView> array_buffer_view) {
  Vector<T> values(base::checked_cast<wtf_size_t>(
      array_buffer_view->byteLength() / array_buffer_view->TypeSize()));
  memcpy(values.data(), array_buffer_view->BaseAddress(),
         array_buffer_view->byteLength());
  return values;
}

template <typename T>
MLOperand* BuildConstant(MLGraphBuilder* builder,
                         const Vector<uint32_t>& dimensions,
                         V8MLOperandDataType::Enum data_type,
                         const Vector<T>& values,
                         ExceptionState& exception_state) {
  size_t buffer_size = std::accumulate(dimensions.begin(), dimensions.end(),
                                       size_t(1), std::multiplies<uint32_t>());
  auto buffer = CreateDOMArrayBufferView(buffer_size, data_type);
  DCHECK_EQ(buffer->byteLength(), values.size() * sizeof(T));
  memcpy(buffer->BaseAddress(), values.data(), buffer->byteLength());
  return BuildConstant(builder, dimensions, data_type, exception_state, buffer);
}

// This method is especially for checking the floating-point output data of some
// ops like the element wise binary pow, unary operator softmax, etc. The output
// data is compared with the expected output data per element by macros
// EXPECT_FLOAT_EQ.
void ExpectFloatArrayEqual(const Vector<float>& data,
                           const Vector<float>& expected_data);

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_TEST_BASE_H_
