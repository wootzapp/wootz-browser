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
#include "base/logging.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/browser/ui/webui/throttle/throttle_prefs.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/throttle_resources.h"
#include "chrome/grit/throttle_resources_map.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
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
#include "net/base/load_flags.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/clear_data_filter.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "ui/resources/grit/webui_resources.h"
#include "url/origin.h"
#include "url/scheme_host_port.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

using content::BrowserThread;
using content::BrowserContext;

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
        void HandleGetNetworkThrottlingSettings(const base::Value::List& args);

        raw_ptr<content::WebUI> web_ui_;
        base::WeakPtrFactory<ThrottleMessageHandler> weak_factory_{this};

        void LoadNetworkThrottlingSettings();
    };

    // This class receives javascript messages from the renderer.
    // Note that the WebUI infrastructure runs on the UI thread, therefore all of
    // this class's methods are expected to run on the UI thread.
    ThrottleMessageHandler::ThrottleMessageHandler(content::WebUI* web_ui)
        : web_ui_(web_ui) {}

    void ThrottleMessageHandler::RegisterMessages() {
        DCHECK_CURRENTLY_ON(BrowserThread::UI);
        web_ui_->RegisterMessageCallback(
            "setNetworkThrottling",
            base::BindRepeating(&ThrottleMessageHandler::OnSetNetworkThrottling,
                                base::Unretained(this)));
         web_ui_->RegisterMessageCallback(
            "getNetworkThrottlingSettings",
            base::BindRepeating(&ThrottleMessageHandler::HandleGetNetworkThrottlingSettings,
                            base::Unretained(this)));                        
    }

    void ThrottleMessageHandler::OnSetNetworkThrottling(
        const base::Value::List& args) {
        DCHECK(args.size() == 6) << "Expected 6 arguments for network throttling settings";

        bool offline = args[0].GetBool();
        double latency = args[1].GetDouble();
        double download_throughput = args[2].GetDouble();
        double upload_throughput = args[3].GetDouble();
        double packet_loss = args[4].GetDouble();
        int packet_queue_length = args[5].GetInt();

        Profile* profile = Profile::FromWebUI(web_ui_);
        PrefService* prefs = profile->GetPrefs();

        if(prefs){

        prefs->SetBoolean(throttle_webui::prefs::kNetworkThrottlingOffline, offline);
        prefs->SetDouble(throttle_webui::prefs::kNetworkThrottlingLatency, latency);
        prefs->SetDouble(throttle_webui::prefs::kNetworkThrottlingDownloadThroughput, download_throughput);
        prefs->SetDouble(throttle_webui::prefs::kNetworkThrottlingUploadThroughput, upload_throughput);
        prefs->SetDouble(throttle_webui::prefs::kNetworkThrottlingPacketLoss, packet_loss);
        prefs->SetInteger(throttle_webui::prefs::kNetworkThrottlingPacketQueueLength, packet_queue_length);

        }
       
        network::mojom::NetworkConditionsPtr conditions = network::mojom::NetworkConditions::New();
        conditions->offline = offline;
        conditions->latency = base::Milliseconds(latency);
        conditions->download_throughput = download_throughput;
        conditions->upload_throughput = upload_throughput;
        conditions->packet_loss = packet_loss;
        conditions->packet_queue_length = packet_queue_length;

        GetNetworkContext()->SetNetworkConditions(devtools_token, std::move(conditions));

        // Notify frontend about the updated settings
        base::Value::List settings;
        settings.Append(offline);
        settings.Append(latency);
        settings.Append(download_throughput);
        settings.Append(upload_throughput);
        settings.Append(packet_loss);
        settings.Append(packet_queue_length);

        AllowJavascript();
        web_ui_->CallJavascriptFunctionUnsafe("displaySavedSettings", settings);
    }
    
    void ThrottleMessageHandler::HandleGetNetworkThrottlingSettings(const base::Value::List& args) {
    Profile* profile = Profile::FromWebUI(web_ui_);
    PrefService* prefs = profile->GetPrefs();

    bool offline = prefs->GetBoolean(throttle_webui::prefs::kNetworkThrottlingOffline);
    double latency = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingLatency);
    double download_throughput = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingDownloadThroughput);
    double upload_throughput = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingUploadThroughput);
    double packet_loss = prefs->GetDouble(throttle_webui::prefs::kNetworkThrottlingPacketLoss);
    int packet_queue_length = prefs->GetInteger(throttle_webui::prefs::kNetworkThrottlingPacketQueueLength);

    base::Value::List settings;
    settings.Append(offline);
    settings.Append(latency);
    settings.Append(download_throughput);
    settings.Append(upload_throughput);
    settings.Append(packet_loss);
    settings.Append(packet_queue_length);

    AllowJavascript();
    web_ui_->CallJavascriptFunctionUnsafe("displaySavedSettings", settings);
}

    void ThrottleMessageHandler::LoadNetworkThrottlingSettings() {
        Profile* profile = Profile::FromWebUI(web_ui_);
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

        GetNetworkContext()->SetNetworkConditions(devtools_token, std::move(conditions));
    }

    network::mojom::NetworkContext* ThrottleMessageHandler::GetNetworkContext() {
        return web_ui_->GetWebContents()
            ->GetBrowserContext()
            ->GetDefaultStoragePartition()
            ->GetNetworkContext();
    }
}  // namespace

ThrottleUI::ThrottleUI(content::WebUI* web_ui)
    : WebUIController(web_ui) {
    web_ui->AddMessageHandler(std::make_unique<ThrottleMessageHandler>(web_ui));

    // Set up the wootzapp://throttle source.
    CreateAndAddThrottleHTMLSource(Profile::FromWebUI(web_ui));

}
