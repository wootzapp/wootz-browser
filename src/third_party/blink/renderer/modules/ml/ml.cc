// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/ml/ml.h"

#include "components/ml/mojom/web_platform_model.mojom-blink.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_ml_context_options.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/ml/buildflags.h"
#include "third_party/blink/renderer/modules/ml/ml_context.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"

namespace blink {

namespace {

using ml::model_loader::mojom::blink::CreateModelLoaderOptionsPtr;
using ml::model_loader::mojom::blink::MLService;

}  // namespace

ML::ML(ExecutionContext* execution_context)
    : ExecutionContextClient(execution_context),
      model_loader_service_(execution_context),
      webnn_context_provider_(execution_context) {}

void ML::CreateModelLoader(ScriptState* script_state,
                           CreateModelLoaderOptionsPtr options,
                           MLService::CreateModelLoaderCallback callback) {
  EnsureModelLoaderServiceConnection(script_state);

  model_loader_service_->CreateModelLoader(std::move(options),
                                           std::move(callback));
}

void ML::CreateWebNNContext(
    webnn::mojom::blink::CreateContextOptionsPtr options,
    webnn::mojom::blink::WebNNContextProvider::CreateWebNNContextCallback
        callback) {
  // Connect WebNN Service if needed.
  EnsureWebNNServiceConnection();

  // Create `WebNNGraph` message pipe with `WebNNContext` mojo interface.
  webnn_context_provider_->CreateWebNNContext(std::move(options),
                                              std::move(callback));
}

void ML::Trace(Visitor* visitor) const {
  visitor->Trace(model_loader_service_);
  visitor->Trace(webnn_context_provider_);
  visitor->Trace(pending_resolvers_);
  ExecutionContextClient::Trace(visitor);
  ScriptWrappable::Trace(visitor);
}

ScriptPromise<MLContext> ML::createContext(ScriptState* script_state,
                                           MLContextOptions* options,
                                           ExceptionState& exception_state) {
  ScopedMLTrace scoped_trace("ML::createContext");
  if (!script_state->ContextIsValid()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Invalid script state");
    return ScriptPromise<MLContext>();
  }

  auto* resolver = MakeGarbageCollected<ScriptPromiseResolver<MLContext>>(
      script_state, exception_state.GetContext());

  auto promise = resolver->Promise();

  MLContext::ValidateAndCreate(resolver, options, this);
  return promise;
}

void ML::RecordPendingResolver(ScriptPromiseResolver<MLContext>* resolver) {
  pending_resolvers_.insert(resolver);
}

void ML::RemovePendingResolver(ScriptPromiseResolver<MLContext>* resolver) {
  pending_resolvers_.erase(resolver);
}

void ML::EnsureModelLoaderServiceConnection(ScriptState* script_state) {
  // The execution context of this navigator is valid here because it has been
  // verified at the beginning of `MLModelLoader::load()` function.
  CHECK(script_state->ContextIsValid());

  // Note that we do not use `ExecutionContext::From(script_state)` because
  // the ScriptState passed in may not be guaranteed to match the execution
  // context associated with this navigator, especially with
  // cross-browsing-context calls.
  if (!model_loader_service_.is_bound()) {
    GetExecutionContext()->GetBrowserInterfaceBroker().GetInterface(
        model_loader_service_.BindNewPipeAndPassReceiver(
            GetExecutionContext()->GetTaskRunner(TaskType::kInternalDefault)));
  }
}

void ML::OnWebNNServiceConnectionError() {
  webnn_context_provider_.reset();

  for (const auto& resolver : pending_resolvers_) {
    resolver->RejectWithDOMException(DOMExceptionCode::kUnknownError,
                                     "WebNN service connection error.");
  }
  pending_resolvers_.clear();
}

void ML::EnsureWebNNServiceConnection() {
  if (webnn_context_provider_.is_bound()) {
    return;
  }
  GetExecutionContext()->GetBrowserInterfaceBroker().GetInterface(
      webnn_context_provider_.BindNewPipeAndPassReceiver(
          GetExecutionContext()->GetTaskRunner(TaskType::kInternalDefault)));
  webnn_context_provider_.set_disconnect_handler(WTF::BindOnce(
      &ML::OnWebNNServiceConnectionError, WrapWeakPersistent(this)));
}

}  // namespace blink
