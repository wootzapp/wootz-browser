// Copyright 2024 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/throttle/throttle_prefs.h"
#include "components/prefs/pref_registry_simple.h"

namespace throttle_webui {

namespace prefs {
    const char kNetworkThrottlingOffline[] = "network_throttling.offline";
    const char kNetworkThrottlingLatency[] = "network_throttling.latency";
    const char kNetworkThrottlingDownloadThroughput[] = "network_throttling.download_throughput";
    const char kNetworkThrottlingUploadThroughput[] = "network_throttling.upload_throughput";
    const char kNetworkThrottlingPacketLoss[] = "network_throttling.packet_loss";
    const char kNetworkThrottlingPacketQueueLength[] = "network_throttling.packet_queue_length";
}  // namespace prefs

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
    registry->RegisterBooleanPref(prefs::kNetworkThrottlingOffline, false);
    registry->RegisterDoublePref(prefs::kNetworkThrottlingLatency, 0.0);
    registry->RegisterDoublePref(prefs::kNetworkThrottlingDownloadThroughput, 0.0);
    registry->RegisterDoublePref(prefs::kNetworkThrottlingUploadThroughput, 0.0);
    registry->RegisterDoublePref(prefs::kNetworkThrottlingPacketLoss, 0.0);
    registry->RegisterIntegerPref(prefs::kNetworkThrottlingPacketQueueLength, 0);
}



 void LoadNetworkThrottlingSettings(Profile* profile) {
        // Profile* profile = Profile::FromWebUI(web_ui_);
        PrefService* prefs = profile->GetPrefs();

        bool offline = prefs->GetBoolean(throttle_webui::prefs::kNetworkThrottlingOffline);
        double latency = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingLatency);
        double download_throughput = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingDownloadThroughput);
        double upload_throughput = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingUploadThroughput);
        double packet_loss = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingPacketLoss);
        int packet_queue_length = prefs->GetInteger(throttle_webui::prefs::kNetworkThrottlingPacketQueueLength);

        network::mojom::NetworkConditionsPtr conditions = network::mojom::NetworkConditions::New();
        conditions->offline = offline;
        conditions->latency = base::Milliseconds(latency);
        conditions->download_throughput = download_throughput;
        conditions->upload_throughput = upload_throughput;
        conditions->packet_loss = packet_loss;
        conditions->packet_queue_length = packet_queue_length;

        GetNetworkContext()->SetNetworkConditions(devtools_token.Create(), std::move(conditions));
    }


}  // namespace throttle_webui
