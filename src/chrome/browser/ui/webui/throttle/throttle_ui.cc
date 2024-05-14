#include "chrome/browser/ui/webui/throttle/throttle_ui.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "base/base64.h"
#include "base/containers/to_value_list.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/i18n/time_formatting.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/common/pref_names.h"
#include "chrome/grit/throttle_resources.h"
#include "chrome/grit/throttle_resources_map.h"
#include "components/prefs/pref_member.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/base/net_errors.h"
#include "net/base/network_isolation_key.h"
#include "net/base/schemeful_site.h"
#include "services/network/public/cpp/request_destination.h"
#include "services/network/public/mojom/clear_data_filter.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "ui/resources/grit/webui_resources.h"
#include "url/origin.h"
#include "url/scheme_host_port.h"

using content::BrowserThread;



namespace {
    void CreateAndAddThrottleHTMLSource(Profile* profile) {
        content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIThrottleHost);
            webui::SetupWebUIDataSource(
            source,
            base::make_span(kThrottleResources, kThrottleResourcesSize),
            IDR_THROTTLE_INDEX_HTML);
            webui::EnableTrustedTypesCSP(source);
   }

    class ThrottleMessageHandler : public content::WebUIMessageHandler {
        public:
            explicit ThrottleMessageHandler(content::WebUI* web_ui);
            
            ThrottleMessageHandler(const ThrottleMessageHandler&) = delete;
            ThrottleMessageHandler& operator=(const ThrottleMessageHandler&) = delete;

            ~ThrottleMessageHandler() override = default;

            protected:
            // WebUIMessageHandler implementation:
            void RegisterMessages() override;
            void OnJavascriptDisallowed() override;

            private:
            network::mojom::NetworkContext* GetNetworkContext();
            const base::UnguessableToken devtools_token;

            void OnSetNetworkThrottling(const base::Value::List& list);
            void ApplySavedNetworkConditions();

            raw_ptr<content::WebUI> web_ui_;
            raw_ptr<profile> profile_;
            base::WeakPtrFactory<ThrottleMessageHandler> weak_factory_{this};
    };

    ThrottleMessageHandler::ThrottleMessageHandler(content::WebUI* web_ui)
        : web_ui_(web_ui) {}

    void ThrottleMessageHandler::RegisterMessages() {
        web_ui_->RegisterMessageCallback(
            "setNetworkThrottling",
            base::BindRepeating(&ThrottleMessageHandler::OnSetNetworkThrottling,
                                base::Unretained(this)));
        ApplySavedNetworkConditions();                        
    }

    void ThrottleMessageHandler::OnJavascriptDisallowed() {
        weak_factory_.InvalidateWeakPtrs();
    }

    void ThrottleMessageHandler::OnSetNetworkThrottling(
        const base::Value::List& args) {
        DCHECK(args.size() == 6) << "Expected 6 arguments for network throttling settings";

        auto* prefs = profile_->GetPrefs();
        prefs->SetBoolean(prefs::kNetworkThrottleOffline, args[0].GetBool());
        prefs->SetDouble(prefs::kNetworkThrottleLatency, args[1].GetDouble());
        prefs->SetDouble(prefs::kNetworkThrottleDownloadThroughput, args[2].GetDouble());
        prefs->SetDouble(prefs::kNetworkThrottleUploadThroughput, args[3].GetDouble());
        prefs->SetDouble(prefs::kNetworkThrottlePacketLoss, args[4].GetDouble());
        prefs->SetInteger(prefs::kNetworkThrottlePacketQueueLength, args[5].GetInt());

        network::mojom::NetworkConditionsPtr conditions = network::mojom::NetworkConditions::New();
        conditions->offline = prefs->GetBoolean(prefs::kNetworkThrottleOffline);
        conditions->latency = base::Milliseconds(prefs->GetDouble(prefs::kNetworkThrottleLatency));
        conditions->download_throughput = prefs->GetDouble(prefs::kNetworkThrottleDownloadThroughput);
        conditions->upload_throughput = prefs->GetDouble(prefs::kNetworkThrottleUploadThroughput);
        conditions->packet_loss = prefs->GetDouble(prefs::kNetworkThrottlePacketLoss);
        conditions->packet_queue_length = prefs->GetInteger(prefs::kNetworkThrottlePacketQueueLength);

        auto* network_context = profile_->GetDefaultStoragePartition()->GetNetworkContext();
        network_context->SetNetworkConditions(base::UnguessableToken::Create(), std::move(conditions));
    }

    void ApplySavedNetworkConditions() {
        auto* prefs = profile_->GetPrefs();
        if (prefs->HasPrefPath(prefs::kNetworkThrottleOffline)) {  // Check if settings exist
            ApplyNetworkConditionsFromPrefs(prefs);
        }
    }

    network::mojom::NetworkContext*
    ThrottleMessageHandler::GetNetworkContext() {
   
    return web_ui_->GetWebContents()
      ->GetBrowserContext()
      ->GetDefaultStoragePartition()
      ->GetNetworkContext();
}

} // namespace


// ThrottleUI

    ThrottleUI::ThrottleUI(content::WebUI* web_ui)
    : WebUIController(web_ui) {
    web_ui->AddMessageHandler(
      std::make_unique<ThrottleMessageHandler>(web_ui));

        // Set up the chrome://net-internals/ source.
        CreateAndAddThrottleHTMLSource(Profile::FromWebUI(web_ui));
    }
