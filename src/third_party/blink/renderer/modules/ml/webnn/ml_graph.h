// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_H_

#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_operand_descriptor.h"
#include "third_party/blink/renderer/modules/ml/ml_trace.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_builder.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_vector.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/heap/visitor.h"
#include "third_party/blink/renderer/platform/wtf/text/string_hash.h"

namespace blink {
class MLBuffer;
class MLComputeResult;
class MLContext;

// Implement the MLNamedArrayBufferViews type definition of WebNN spec:
// https://www.w3.org/TR/webnn/#typedefdef-mlnamedarraybufferviews
typedef HeapVector<std::pair<String, NotShared<DOMArrayBufferView>>>
    MLNamedArrayBufferViews;

typedef HeapVector<std::pair<String, Member<MLBuffer>>> MLNamedBuffers;

class MODULES_EXPORT MLGraph : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  MLGraph(const MLGraph&) = delete;
  MLGraph& operator=(const MLGraph&) = delete;

  ~MLGraph() override;

  void Trace(Visitor* visitor) const override;

  // The members of ResourceInfo are used to validate the inputs and outputs of
  // an MLGraph execution. The validation steps are described by WebNN spec of
  // the MLContext.compute() method:
  // https://www.w3.org/TR/webnn/#api-mlcontext-async-execution
  // The plain struct ResourceInfo is introduced instead of using
  // MLOperandDescriptor because neither byte length calculation from dimensions
  // nor GC support is needed for the implementation.
  struct ResourceInfo {
    V8MLOperandDataType::Enum data_type;
    size_t byte_length;
  };
  const HashMap<String, ResourceInfo>& GetInputResourcesInfo() const;
  const HashMap<String, ResourceInfo>& GetOutputResourcesInfo() const;

  // This method validates the input and output MLNamedArrayBufferViews against
  // the graph's input and output resources info. If there are no errors, it
  // transfers the input and output ArrayBufferViews to new ones and passes them
  // ones to ComputeImpl() implemented by an MLGraph backend that binds the
  // array buffer views and executes the compiled platform graph. This method is
  // called by MLContext to implement MLContext.compute() method.
  void Compute(ScopedMLTrace scoped_trace,
               const MLNamedArrayBufferViews& inputs,
               const MLNamedArrayBufferViews& outputs,
               ScriptPromiseResolver<MLComputeResult>* resolver,
               ExceptionState& exception_state);

  // This method validates the input and output MLNamedBuffers against
  // the graph's input and output resources info. If there are no errors, it
  // passes the buffers to DispatchImpl() implemented by an MLGraph backend that
  // binds the buffers and executes the compiled platform graph.
  // This method is called by MLContext to implement MLContext.dispatch()
  // method.
  void Dispatch(ScopedMLTrace scoped_trace,
                const MLNamedBuffers& inputs,
                const MLNamedBuffers& outputs,
                ExceptionState& exception_state);

  const MLContext* Context() const;

 protected:
  explicit MLGraph(MLContext* context);

  // Build() should be called right after constructing a concrete
  // MLGraph object. Build() validates the named outputs and initializes
  // the input and output resources info. If there are no errors, it calls
  // BuildImpl() implemented by an MLGraph backend that builds the platform
  // specific graph.
  void Build(ScopedMLTrace scoped_trace,
             const MLNamedOperands& named_outputs,
             ScriptPromiseResolver<MLGraph>* resolver);

  // An MLGraph backend should implement this method to build and compile a
  // platform specific graph asynchronously. The actual graph construction and
  // compilation work should be handled by a worker thread without blocking the
  // main thread. Once the platform graph is compiled, the resolver should be
  // resolved with a concrete MLGraph object. Otherwise, the resolver should be
  // rejected with a DOMException accordingly.
  virtual void BuildImpl(ScopedMLTrace scoped_trace,
                         const MLNamedOperands& outputs,
                         ScriptPromiseResolver<MLGraph>* resolver) = 0;

  // An MLGraph backend should implement this method to execute the compiled
  // platform graph asynchronously. The actual graph execution work should be
  // handled by a worker thread without blocking the main thread.
  //
  // The implementation should transfer the input and output
  // `MLNamedArrayBufferViews` to new views that share the same backing memory
  // allocations.
  //
  // If compute is successful, the results will be stored in output buffers and
  // the resolver will be resolved with an MLComputeResult that contains the
  // input and output buffers. Otherwise, the resolver will be rejected with a
  // DOMException accordingly.
  virtual void ComputeImpl(ScopedMLTrace scoped_trace,
                           const MLNamedArrayBufferViews& inputs,
                           const MLNamedArrayBufferViews& outputs,
                           ScriptPromiseResolver<MLComputeResult>* resolver,
                           ExceptionState& exception_state) = 0;

  virtual void DispatchImpl(ScopedMLTrace scoped_trace,
                            const MLNamedBuffers& inputs,
                            const MLNamedBuffers& outputs,
                            ExceptionState& exception_state) = 0;

  Member<MLContext> ml_context_;
  bool resources_info_initialized_{false};
  HashMap<String, ResourceInfo> input_resources_info_;
  HashMap<String, ResourceInfo> output_resources_info_;

 private:
  // This helper method is called by Build(). It validates named outputs
  // and initializes the input and output resources info by graph traversal.
  base::expected<void, String> ValidateAndInitializeResourcesInfo(
      const MLNamedOperands& named_outputs);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_H_
