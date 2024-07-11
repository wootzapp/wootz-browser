#include "chrome/browser/ui/webui/hello_world/hello_world_ui.h"

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
#include "chrome/browser/ui/webui/hello_world/hello_world_prefs.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/hello_world_resources.h"
#include "chrome/grit/hello_world_resources_map.h"
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

void CreateAndAddHelloWorldHTMLSource(Profile* profile) {
    content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
        profile, chrome::kChromeUIHelloWorldHost);
    webui::SetupWebUIDataSource(
        source,
        base::make_span(kHelloWorldResources, kHelloWorldResourcesSize),
        IDR_HELLO_WORLD_HELLO_WORLD_HTML);
    webui::EnableTrustedTypesCSP(source);

    source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc,
      "connect-src 'self'  wootzapp://resources wootzapp://theme;");
}


class HelloWorldMessageHandler : public content::WebUIMessageHandler {
public:
    explicit HelloWorldMessageHandler(content::WebUI* web_ui);

    HelloWorldMessageHandler(const HelloWorldMessageHandler&) = delete;
    HelloWorldMessageHandler& operator=(const HelloWorldMessageHandler&) = delete;

    ~HelloWorldMessageHandler() override = default;

protected:
    // WebUIMessageHandler implementation:
    void RegisterMessages() override;
    void OnJavascriptDisallowed() override;

private:
    void LoginWallet(const base::Value::List& args);
    void LogUrl(const base::Value::List& args);
    void Encrypt(const base::Value::List& args);
    void GetUserProfile(const base::Value::List& args);
    void OnURLLoadComplete(std::unique_ptr<std::string> response_body);

    raw_ptr<content::WebUI> web_ui_;
    std::unique_ptr<network::SimpleURLLoader> simple_loader;
    base::WeakPtrFactory<HelloWorldMessageHandler> weak_factory_{this};
};

HelloWorldMessageHandler::HelloWorldMessageHandler(content::WebUI* web_ui)
    : web_ui_(web_ui) {}

void HelloWorldMessageHandler::RegisterMessages() {
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    web_ui_->RegisterMessageCallback(
        "loginWallet",
        base::BindRepeating(&HelloWorldMessageHandler::LoginWallet,
                            base::Unretained(this))); 
    web_ui_->RegisterMessageCallback(
        "logUrl",
        base::BindRepeating(&HelloWorldMessageHandler::LogUrl,
                            base::Unretained(this)));
    web_ui_->RegisterMessageCallback(
        "encrypt",
        base::BindRepeating(&HelloWorldMessageHandler::Encrypt,
                            base::Unretained(this)));
    web_ui_->RegisterMessageCallback(
        "getUserProfile",
        base::BindRepeating(&HelloWorldMessageHandler::GetUserProfile,
                            base::Unretained(this)));
}

void HelloWorldMessageHandler::OnJavascriptDisallowed() {
    weak_factory_.InvalidateWeakPtrs();
}

void HelloWorldMessageHandler::LoginWallet(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/users/authentication/signin");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR)<< "HELLLOO : login"<<" ";
    Profile* profile = Profile::FromWebUI(web_ui_);

    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
      profile->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();

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
      base::BindOnce(&HelloWorldMessageHandler::OnURLLoadComplete,
                     base::Unretained(this)),
      1024 * 1024);
}

void HelloWorldMessageHandler::LogUrl(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/logs/url");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR)<< "Logging URL"<<" ";
    Profile* profile = Profile::FromWebUI(web_ui_);

    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
      profile->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();

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

    std::string token = args[0].GetString();
    std::string url = args[1].GetString();

    std::string request_body = R"({"encrypted_url":")" + url + R"("})";
    simple_loader->AttachStringForUpload(request_body, "application/json");

    simple_loader->DownloadToString(
      loader_factory.get(),
      base::BindOnce(&HelloWorldMessageHandler::OnURLLoadComplete,
                     base::Unretained(this)),
      1024 * 1024);
}

void HelloWorldMessageHandler::Encrypt(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/encrypt");
    resource_request->method = "POST";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR)<< "Encrypting Data"<<" ";
    Profile* profile = Profile::FromWebUI(web_ui_);

    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
      profile->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();

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

    std::string token = args[0].GetString();
    std::string secret = args[1].GetString();
    std::string data = args[2].GetString();

    std::string request_body = R"({"rawData":")" + data + R"(","secretKey":")" + secret + R"("})";
    simple_loader->AttachStringForUpload(request_body, "application/json");

    simple_loader->DownloadToString(
      loader_factory.get(),
      base::BindOnce(&HelloWorldMessageHandler::OnURLLoadComplete,
                     base::Unretained(this)),
      1024 * 1024);
}

void HelloWorldMessageHandler::GetUserProfile(const base::Value::List& args) {
    auto resource_request = std::make_unique<network::ResourceRequest>();
    resource_request->url = GURL("https://api-staging-0.gotartifact.com/v2/users/me");
    resource_request->method = "GET";

    resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
    resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

    LOG(ERROR)<< "Getting User Profile"<<" ";
    Profile* profile = Profile::FromWebUI(web_ui_);

    scoped_refptr<network::SharedURLLoaderFactory> loader_factory =
      profile->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();

    net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("artifact_api_profile", R"(
        semantics {
          sender: "Artifact API"
          description: "Getting user profile."
          trigger: "Whenever the network throttling settings are changed."
          data: "None."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

    simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

    std::string token = args[0].GetString();
    resource_request->headers.SetHeader("Authorization", "Bearer " + token);

    simple_loader->DownloadToString(
      loader_factory.get(),
      base::BindOnce(&HelloWorldMessageHandler::OnURLLoadComplete,
                     base::Unretained(this)),
      1024 * 1024);
}

void HelloWorldMessageHandler::OnURLLoadComplete(
    std::unique_ptr<std::string> response_body) {
  std::string data = response_body ? std::move(*response_body) : "";

  LOG(ERROR)<< "Response from API: "<<" ";
  LOG(ERROR) << data;
}

}  // namespace

HelloWorldUI::HelloWorldUI(content::WebUI* web_ui)
    : WebUIController(web_ui) {
    web_ui->AddMessageHandler(std::make_unique<HelloWorldMessageHandler>(web_ui));

    // Set up the wootzapp://hello_world source.
    CreateAndAddHelloWorldHTMLSource(Profile::FromWebUI(web_ui));
}
