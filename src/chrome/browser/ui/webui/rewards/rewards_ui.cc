#include "chrome/browser/ui/webui/rewards/rewards_ui.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "base/base64.h"
#include "base/containers/to_value_list.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/command_line.h"
#include "base/json/json_string_value_serializer.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/i18n/time_formatting.h"
#include "base/logging.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "chrome/browser/prefs/browser_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "chrome/browser/history/history_service_factory.h"
#include "components/history/core/browser/history_service.h"
#include "components/history/core/browser/history_types.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/rewards_page_resources.h"
#include "chrome/grit/rewards_page_resources_map.h"
#include "components/prefs/pref_member.h"
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
#include "net/base/url_util.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "net/http/http_response_headers.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/clear_data_filter.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "ui/resources/grit/webui_resources.h"
#include "url/origin.h"
#include "url/scheme_host_port.h"

using content::BrowserThread;
using content::BrowserContext;

namespace {

void CreateAndAddRewardsHTMLSource(Profile* profile) {
    content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
        profile, chrome::kChromeUIRewardsHost);
    webui::SetupWebUIDataSource(
      source,
      base::make_span(kRewardsPageResources, kRewardsPageResourcesSize),
      IDR_REWARDS_PAGE);

    source->DisableTrustedTypesCSP();
  
    source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc, "connect-src *;");
      
    source->AddString("message", "Hello World!");
}

}  // namespace

RewardsMessageHandler::RewardsMessageHandler(content::WebUI* web_ui)
    : web_ui_(web_ui), weak_factory_(this) {
    Profile* profile = Profile::FromWebUI(web_ui_);
    history::HistoryService* history_service =
        HistoryServiceFactory::GetForProfile(profile, ServiceAccessType::EXPLICIT_ACCESS);
    if (history_service) {
        history_service->AddObserver(this);
    }
}

RewardsMessageHandler::~RewardsMessageHandler() {
    Profile* profile = Profile::FromWebUI(web_ui_);
    history::HistoryService* history_service =
        HistoryServiceFactory::GetForProfile(profile, ServiceAccessType::EXPLICIT_ACCESS);
    if (history_service) {
        history_service->RemoveObserver(this);
    }
}

void RewardsMessageHandler::RegisterMessages() {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    web_ui_->RegisterMessageCallback(
        "loginWallet",
        base::BindRepeating(&RewardsMessageHandler::LoginWallet,
                            base::Unretained(this)));
    web_ui_->RegisterMessageCallback(
        "logUrl",
        base::BindRepeating(&RewardsMessageHandler::LogUrl,
                            base::Unretained(this)));
    web_ui_->RegisterMessageCallback(
        "encrypt",
        base::BindRepeating(&RewardsMessageHandler::Encrypt,
                            base::Unretained(this)));
    web_ui_->RegisterMessageCallback(
        "getUserProfile",
        base::BindRepeating(&RewardsMessageHandler::GetUserProfile,
                            base::Unretained(this)));
}

void RewardsMessageHandler::OnJavascriptDisallowed() {
    weak_factory_.InvalidateWeakPtrs();
}

void RewardsMessageHandler::LoginWallet(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/users/authentication/signin");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                   net::LOAD_DISABLE_CACHE |
                                   net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR) << "HELLLOO : login";

    Profile* profile = Profile::FromWebUI(web_ui_);
    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
        profile->GetDefaultStoragePartition()->GetURLLoaderFactoryForBrowserProcess();

    net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("artifact_api", R"(
        semantics {
          sender: "Artifact API"
          description: "Accessing Artifact API to authenticate a user."
          trigger: "Whenever the network throttling settings are changed."
          data: "Email and password for authentication."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

    simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

    std::string request_body = R"({"email":")" + args[0].GetString() + R"(","password":")" + args[1].GetString() + R"("})";
    simple_loader->AttachStringForUpload(request_body, "application/json");

    simple_loader->DownloadToString(
        loader_factory.get(),
        base::BindOnce(&RewardsMessageHandler::OnURLLoadComplete,
                       base::Unretained(this)),
        1024 * 1024);
}

void RewardsMessageHandler::LogUrl(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/logs/url");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                   net::LOAD_DISABLE_CACHE |
                                   net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR) << "Logging URL";

    Profile* profile = Profile::FromWebUI(web_ui_);
    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
        profile->GetDefaultStoragePartition()->GetURLLoaderFactoryForBrowserProcess();

    net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("artifact_api_log", R"(
        semantics {
          sender: "Artifact API"
          description: "Logging a URL."
          trigger: "Whenever the network throttling settings are changed."
          data: "Encrypted URL."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

    simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

    std::string url = args[1].GetString();
    std::string request_body = R"({"encrypted_url":")" + url + R"("})";
    simple_loader->AttachStringForUpload(request_body, "application/json");

    simple_loader->DownloadToString(
        loader_factory.get(),
        base::BindOnce(&RewardsMessageHandler::OnURLLoadComplete,
                       base::Unretained(this)),
        1024 * 1024);
}

void RewardsMessageHandler::Encrypt(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/encrypt");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                   net::LOAD_DISABLE_CACHE |
                                   net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR) << "Encrypting Data";

    Profile* profile = Profile::FromWebUI(web_ui_);
    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
        profile->GetDefaultStoragePartition()->GetURLLoaderFactoryForBrowserProcess();

    net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("artifact_api_encrypt", R"(
        semantics {
          sender: "Artifact API"
          description: "Encrypting data."
          trigger: "Whenever the network throttling settings are changed."
          data: "Data and secret for encryption."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

    simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

    std::string secret = args[1].GetString();
    std::string data = args[2].GetString();
    std::string request_body = R"({"rawData":")" + data + R"(","secretKey":")" + secret + R"("})";
    simple_loader->AttachStringForUpload(request_body, "application/json");

    simple_loader->DownloadToString(
        loader_factory.get(),
        base::BindOnce(&RewardsMessageHandler::OnURLLoadComplete,
                       base::Unretained(this)),
        1024 * 1024);
}

void RewardsMessageHandler::SendStoredApiResponse() {
    // Profile* profile = Profile::FromWebUI(web_ui_);
    // PrefService* prefs = profile->GetPrefs();
   std::string stored_response = R"({"success": true, "data": {"id_token": "eyJhbGciOiJSUzI1NiIsImtpZCI6ImMxNTQwYWM3MWJiOTJhYTA2OTNjODI3MTkwYWNhYmU1YjA1NWNiZWMiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2dsZS5jb20vYWR0ZXN0LTk2YWJlIiwiYXVkIjoiYWR0ZXN0LTk2YWJlIiwiYXV0aF90aW1lIjoxNzIxMzI5OTI3LCJ1c2VyX2lkIjoiR2VwRm1wVE5qQ1hIcVNzeXJJcW5hVmowUUU2MiIsInN1YiI6IkdlcEZtcFROakNYSHFTc3lySXFuYVZqMFFFNjIiLCJpYXQiOjE3MjEzMjk5MjcsImV4cCI6MTcyMTMzMzUyNywiZW1haWwiOiJqYXlhZG1pbkBnbWFpbC5jb20iLCJlbWFpbF92ZXJpZmllZCI6ZmFsc2UsImZpcmViYXNlIjp7ImlkZW50aXRpZXMiOnsiZW1haWwiOlsiamF5YWRtaW5AZ21haWwuY29tIl19LCJzaWduX2luX3Byb3ZpZGVyIjoicGFzc3dvcmQifX0.nEYxsJ0c_-FQzTO3nQGLziZtLUIFayZko8AZaY_A7pMGvHGNWRO_jE4-p5khpmoQahqI3DZoEp6t_9IoMPxJqR8kpUu5AWfyegj7ZRfqqRYPjJ9bL8TTNpG9r6iUS_ILKi-8N1IfLUfTUUe7snkrkdHS7HMEMT8ouVimyyD_Mz2B7Ujtm-D6r4r-6m7_UK3uKAoqisTASrv4t8p73dOKLLHjslcT02A20J11xDwJdlgsNOBlDfIOI6heOMP640HhV3BtoB-Oe9xLwbDbSkeJcjXHBHNeTxaIhFlccnD-9NwVjbrt_cPJgij1WPQIvfsBE7cff9nCSBR0VNCZKlII5w"}})";
    
    if (!stored_response.empty()) {
        base::Value response(stored_response);
        web_ui_->CallJavascriptFunctionUnsafe("handleResponse", response);
    }
}

void RewardsMessageHandler::GetUserProfile(const base::Value::List& args) {
    // Check if the args contain the expected token
    if (args.empty() || !args[0].is_string()) {
        LOG(ERROR) << "Invalid arguments for GetUserProfile";
        return;
    }

    std::string token = args[0].GetString();
    LOG(ERROR) << "HELLOO 1: Token - " << token;

    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/users/me");
    resource_request->method = "GET";
    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                   net::LOAD_DISABLE_CACHE |
                                   net::LOAD_DO_NOT_SAVE_COOKIES;

    resource_request->headers.SetHeader(net::HttpRequestHeaders::kAuthorization, "Bearer " + token);
    LOG(ERROR) << "HELLOO 2: Headers set";

    Profile* profile = Profile::FromWebUI(web_ui_);
    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
        profile->GetDefaultStoragePartition()->GetURLLoaderFactoryForBrowserProcess();

    net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("artifact_api_profile", R"(
        semantics {
          sender: "Artifact API"
          description: "Getting user profile."
          trigger: "User requests their profile."
          data: "None."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

    simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

    simple_loader->DownloadToString(
        loader_factory.get(),
        base::BindOnce(&RewardsMessageHandler::OnGetUserProfileComplete,
                       base::Unretained(this)),
        1024 * 1024);

    LOG(ERROR) << "HELLOO 3: Request sent";
}

void RewardsMessageHandler::OnGetUserProfileComplete(
    std::unique_ptr<std::string> response_body) {
    std::string data = response_body ? std::move(*response_body) : "";
    LOG(ERROR) << "Response from GetUserProfile API: " << data;

    // Send the response back to the frontend
    base::Value response(data);
    web_ui_->CallJavascriptFunctionUnsafe("handleProfileResponse", response);
}


void RewardsMessageHandler::OnURLLoadComplete(
    std::unique_ptr<std::string> response_body) {
    std::string data = response_body ? std::move(*response_body) : "";

    LOG(ERROR) << "Response from API: " << data;

    // Profile* profile = Profile::FromWebUI(web_ui_);
    // PrefService* prefs = profile->GetPrefs();
    // prefs->SetString(prefs::kRewardsApiResponse, data);

    base::Value response(data);
    web_ui_->CallJavascriptFunctionUnsafe("handleResponse", response);
}

void RewardsMessageHandler::OnURLVisited(history::HistoryService* history_service,
                                         const history::URLRow& url_row,
                                         const history::VisitRow& visit_row) {
    std::string latest_url = url_row.url().spec();
    LOG(ERROR) << "Latest URL visited: " << latest_url;

    // Call the function to send the stored API response to the frontend
    SendStoredApiResponse();
}

void RewardsMessageHandler::RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
    registry->RegisterStringPref(prefs::kRewardsApiResponse, std::string("api_response"));
}

RewardsUI::RewardsUI(content::WebUI* web_ui)
    : WebUIController(web_ui) {
    web_ui->AddMessageHandler(std::make_unique<RewardsMessageHandler>(web_ui));

    // Set up the wootzapp://rewards source.
    CreateAndAddRewardsHTMLSource(Profile::FromWebUI(web_ui));
}
