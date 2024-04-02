// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_MODEL_LOADER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_MODEL_LOADER_H_

#include "components/ml/mojom/web_platform_model.mojom-blink.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/modules/ml/ml_trace.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_graph_utils.h"
#include "third_party/blink/renderer/modules/ml/webnn/ml_operand.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/heap/visitor.h"
#include "third_party/blink/renderer/platform/mojo/heap_mojo_remote.h"
#include "third_party/flatbuffers/src/include/flatbuffers/flatbuffers.h"

namespace blink {

// Provides a mechanism to delegate an ML model (as a format-agnostic blob) to
// the browser for inferencing.
//
// TODO(https://crbug.com/1513481): Currently this handles only TFLite models.
// Consider moving the TFLite-specific logic in this class somewhere else. If it
// is later decided that this class will only handle TFLite models, rename it
// appropriately.
class MODULES_EXPORT MLGraphModelLoader final : public MLGraph {
 public:
  // Create and build an MLGraphModelLoader object. Resolve the promise with
  // this concrete object if the underlying TF-Lite model converted from WebNN
  // graph builds successfully.
  // The caller must call `Promise()` on `resolver` before calling this method.
  static void ValidateAndBuild(ScopedMLTrace scoped_trace,
                               MLContext* context,
                               const MLNamedOperands& named_outputs,
                               ScriptPromiseResolverTyped<MLGraph>* resolver);

  // The constructor shouldn't be called directly, use ValidateAndBuild()
  // method instead, and the declaration must be public to be called by
  // MakeGarbageCollected.
  MLGraphModelLoader(ExecutionContext* execution_context, MLContext* context);
  ~MLGraphModelLoader() override;

  void Trace(Visitor* visitor) const override;

  // The caller of this function is responsible to keep flatbuffer alive and
  // unset it when it's no longer used.
  static void SetFlatbufferForTesting(flatbuffers::DetachedBuffer* flatbuffer);

 private:
  // The callback of loading tflite model, it will bind the `Model` pending
  // remote if it's successful.
  void OnRemoteModelLoad(
      ScopedMLTrace scoped_trace,
      ExecutionContext* execution_context,
      ScriptPromiseResolverTyped<MLGraph>* resolver,
      ml::model_loader::mojom::blink::LoadModelResult result,
      mojo::PendingRemote<ml::model_loader::mojom::blink::Model> pending_remote,
      ml::model_loader::mojom::blink::ModelInfoPtr model_info);

  // Load a WebNN graph in `MLService` with `ModelLoader` message pipe, the
  // operations of WebNN need to be converted into a TF-Lite model in
  // FlatBuffers.
  void BuildImpl(ScopedMLTrace scoped_trace,
                 const MLNamedOperands& named_outputs,
                 ScriptPromiseResolverTyped<MLGraph>* resolver) override;

  // Compute the converted model with asynchronous call of `Model` interface.
  void ComputeImpl(ScopedMLTrace scoped_trace,
                   const MLNamedArrayBufferViews& inputs,
                   const MLNamedArrayBufferViews& outputs,
                   ScriptPromiseResolverTyped<MLComputeResult>* resolver,
                   ExceptionState& exception_state) override;

  // Resolve the promise with an MLComputeResult that contains input and output
  // ArrayBufferViews. The `inputs_info` and `outputs_info` carry the backing
  // memory in `ArrayBufferContents` transferred from the original user supplied
  // `ArrayBufferView`s.
  void OnComputeGraph(
      ScopedMLTrace scoped_trace,
      ScriptPromiseResolverTyped<MLComputeResult>* resolver,
      std::unique_ptr<Vector<std::pair<String, ArrayBufferViewInfo>>>
          inputs_info,
      std::unique_ptr<Vector<std::pair<String, ArrayBufferViewInfo>>>
          outputs_info,
      ml::model_loader::mojom::blink::ComputeResult mojo_result,
      const std::optional<HashMap<String, Vector<uint8_t>>>& mojo_outputs);

  HeapMojoRemote<ml::model_loader::mojom::blink::Model> remote_model_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_ML_WEBNN_ML_GRAPH_MODEL_LOADER_H_
