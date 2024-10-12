// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_WEBNN_WEBNN_CONTEXT_IMPL_H_
#define SERVICES_WEBNN_WEBNN_CONTEXT_IMPL_H_

#include "base/component_export.h"
#include "base/containers/flat_set.h"
#include "base/types/optional_ref.h"
#include "mojo/public/cpp/base/big_buffer.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/unique_associated_receiver_set.h"
#include "services/webnn/public/mojom/webnn_buffer.mojom.h"
#include "services/webnn/public/mojom/webnn_context_provider.mojom.h"
#include "services/webnn/public/mojom/webnn_graph.mojom.h"
#include "services/webnn/webnn_object_impl.h"

namespace webnn {

class WebNNBufferImpl;
class WebNNContextProviderImpl;
class WebNNGraphImpl;

class COMPONENT_EXPORT(WEBNN_SERVICE) WebNNContextImpl
    : public mojom::WebNNContext {
 public:
  WebNNContextImpl(mojo::PendingReceiver<mojom::WebNNContext> receiver,
                   WebNNContextProviderImpl* context_provider);

  WebNNContextImpl(const WebNNContextImpl&) = delete;
  WebNNContextImpl& operator=(const WebNNContextImpl&) = delete;

  ~WebNNContextImpl() override;

  // Disassociates a `WebNNBuffer` instance owned by this context by its handle.
  // Called when a `WebNNBuffer` instance has a connection error. After this
  // call, it is no longer safe to use the WebNNBufferImpl.
  void DisconnectAndDestroyWebNNBufferImpl(
      const base::UnguessableToken& handle);

  // This method will be called once `WebNNGraph::CreateGraph()` completes
  // initialization to associate the `WebNNGraph` instance and receiver to
  // this context. Once called, the `WebNNGraph` instance can safely access the
  // `WebNNContext` instance in graph operations.
  void OnWebNNGraphImplCreated(
      mojo::PendingAssociatedReceiver<mojom::WebNNGraph> receiver,
      std::unique_ptr<WebNNGraphImpl> graph_impl);

  // Retrieves a `WebNNBufferImpl` instance created from this context.
  // Emits a bad message if a buffer with the given handle does not exist.
  base::optional_ref<WebNNBufferImpl> GetWebNNBufferImpl(
      const base::UnguessableToken& handle);

 protected:
  void OnConnectionError();

  // mojom::WebNNContext
  void CreateGraph(mojom::GraphInfoPtr graph_info,
                   CreateGraphCallback callback) override;

  void CreateBuffer(
      mojo::PendingAssociatedReceiver<mojom::WebNNBuffer> receiver,
      mojom::BufferInfoPtr buffer_info,
      const base::UnguessableToken& buffer_handle) override;

  // This method will be called by `CreateGraph()` after the graph info is
  // validated. A backend subclass should implement this method to build and
  // compile a platform specific graph asynchronously.
  virtual void CreateGraphImpl(mojom::GraphInfoPtr graph_info,
                               CreateGraphCallback callback) = 0;

  // This method will be called by `CreateBuffer()` after the buffer info is
  // validated. A backend subclass should implement this method to create and
  // initialize a platform specific buffer.
  virtual std::unique_ptr<WebNNBufferImpl> CreateBufferImpl(
      mojo::PendingAssociatedReceiver<mojom::WebNNBuffer> receiver,
      mojom::BufferInfoPtr buffer_info,
      const base::UnguessableToken& buffer_handle) = 0;

  mojo::Receiver<mojom::WebNNContext> receiver_;

  // Owns this object.
  raw_ptr<WebNNContextProviderImpl> context_provider_;

  // BufferImpls must be stored on the context to allow the WebNN service to
  // identify and use them from the renderer process in MLContext operations.
  // This cache only contains valid BufferImpls whose size is managed by the
  // lifetime of the buffers it contains.
  base::flat_set<std::unique_ptr<WebNNBufferImpl>,
                 WebNNObjectImpl::Comparator<WebNNBufferImpl>>
      buffer_impls_;

 private:
  // GraphsImpls which are stored on the context to allow graph
  // operations to use this context safely via a raw_ptr.
  mojo::UniqueAssociatedReceiverSet<mojom::WebNNGraph> graph_impls_;
};

}  // namespace webnn

#endif  // SERVICES_WEBNN_WEBNN_CONTEXT_IMPL_H_
