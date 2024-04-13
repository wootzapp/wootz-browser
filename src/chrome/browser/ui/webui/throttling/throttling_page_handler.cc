// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/throttling/throttling_page_handler.h"
#include "services/network/public/mojom/network_context.mojom.h"

#include "base/functional/bind.h"
#include "chrome/browser/profiles/profile.h"

ThrottlingPageHandler::ThrottlingPageHandler( 
    mojo::PendingReceiver<throttling::mojom::PageHandlerFactory> receiver,
    mojo::PendingRemote<throttling::mojom::Page> page,
    Profile* profile)
    : receiver_(this, std::move(receiver)),
       page_(std::move(page))
    {}

ThrottlingPageHandler::~ThrottlingPageHandler() { 

}

void ThrottlingPageHandler::SetCustomNetworkConditions(bool offline,
    double latency,
    double download_throughput,
    double upload_throughput,
    Maybe<protocol::Network::ConnectionType>,
    Maybe<double> packet_loss,
    Maybe<int> packet_queue_length,
    Maybe<bool> packet_reordering){

    network_conditions = network::mojom::NetworkContext::New();
    network_conditions->offline = offline;
    network_conditions->latency = base::Milliseconds(latency);
    network_conditions->download_throughput = download_throughput;
    network_conditions->upload_throughput = upload_throughput;
    network_conditions->packet_loss = packet_loss.value_or(0.);
    network_conditions->packet_queue_length = packet_queue_length.value_or(0);
    network_conditions->packet_reordering = packet_reordering.value_or(false);


    if (!storage_partition_)
    return;
    network::mojom::NetworkContext* context =
      storage_partition_->GetNetworkContext();

    if (!devtools_token_.is_empty())
    context->SetNetworkConditions(devtools_token_, std::move(network_conditions));

}