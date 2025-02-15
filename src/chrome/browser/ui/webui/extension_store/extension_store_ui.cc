#include "chrome/browser/ui/webui/extension_store/extension_store_ui.h"

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/extension_store_page_resources.h"
#include "chrome/grit/extension_store_page_resources_map.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

ExtensionStoreUI::ExtensionStoreUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui), content::WebUIMessageHandler() {
  // Set up the data source for the WebUI.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIExtensionStoreHost);

  // Add required resources.
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kExtensionStorePageResources, kExtensionStorePageResourcesSize),
      IDR_EXTENSION_STORE_PAGE);

  // Override CSP to allow fetching data from GitHub.
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc,
      "connect-src 'self' https://raw.githubusercontent.com;");

  source->AddString("message", "Hello World!");

  // Register message handlers.
  web_ui->AddMessageHandler(base::WrapUnique(this));
}

ExtensionStoreUI::~ExtensionStoreUI() = default;

void ExtensionStoreUI::RegisterMessages() {
  // Explicitly use WebUIController's web_ui() method.
  content::WebUIController::web_ui()->RegisterMessageCallback(
      "fetchExtensions",
      base::BindRepeating(&ExtensionStoreUI::HandleFetchExtensions,
                          base::Unretained(this)));
}

void ExtensionStoreUI::HandleFetchExtensions(const base::Value::List& args) {
  LOG(ERROR) << "Starting to fetch extensions...";
  
  auto resource_request = std::make_unique<network::ResourceRequest>();
  GURL fetch_url("https://raw.githubusercontent.com/itskartike910/extensions/main/extensions.json");
  
  if (!fetch_url.is_valid()) {
    LOG(ERROR) << "Invalid URL: " << fetch_url.spec();
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Invalid URL"));
    return;
  }
  
  // Verify URL scheme
  if (!fetch_url.SchemeIs("https")) {
    LOG(ERROR) << "Invalid URL scheme: " << fetch_url.scheme();
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Invalid URL scheme"));
    return;
  }
  
  resource_request->url = fetch_url;
  LOG(ERROR) << "Attempting to fetch from URL: " << resource_request->url.spec();
  
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  
  // Add additional headers to help with debugging
  resource_request->headers.SetHeader("Accept", "application/json");
  resource_request->headers.SetHeader("User-Agent", "Chrome Extension Store");
  
  LOG(ERROR) << "Request headers set";

  net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation(
      "extension_store_fetch_extensions", R"(
        semantics {
          sender: "Extension Store"
          description: "Fetching extensions list from GitHub."
          trigger: "User opens the extension store."
          data: "None."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

  auto loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);
  
  // Fix: Use only RETRY_ON_NETWORK_CHANGE as RETRY_ON_SERVER_ERROR is not available
  loader->SetRetryOptions(
      3,  // max retries
      network::SimpleURLLoader::RetryMode::RETRY_ON_NETWORK_CHANGE);

  // Add response started callback
  loader->SetOnResponseStartedCallback(base::BindOnce(
      [](const GURL& url, const network::mojom::URLResponseHead& response_head) {
        LOG(ERROR) << "Response started for URL: " << url.spec();
        if (response_head.headers) {
          LOG(ERROR) << "Response code: " << response_head.headers->response_code();
          LOG(ERROR) << "Headers: " << response_head.headers->raw_headers();
        }
      }));

  // Add download progress callback
  loader->SetOnDownloadProgressCallback(base::BindRepeating(
      [](uint64_t current) {
        LOG(ERROR) << "Download progress: " << current << " bytes";
      }));

  content::BrowserContext* browser_context =
      content::WebUIController::web_ui()->GetWebContents()->GetBrowserContext();
  
  if (!browser_context) {
    LOG(ERROR) << "Failed to get browser context";
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Browser context error"));
    return;
  }
  LOG(ERROR) << "Browser context obtained";

  auto url_loader_factory = browser_context->GetDefaultStoragePartition()
                               ->GetURLLoaderFactoryForBrowserProcess();
  
  if (!url_loader_factory) {
    LOG(ERROR) << "Failed to get URL loader factory";
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("URL loader factory error"));
    return;
  }
  LOG(ERROR) << "URL loader factory obtained";

  LOG(ERROR) << "Initiating download...";
  loader->DownloadToString(
      url_loader_factory.get(),
      base::BindOnce(&ExtensionStoreUI::OnFetchExtensionsComplete,
                     weak_factory_.GetWeakPtr()),
      1024 * 1024);  // 1MB max size
  LOG(ERROR) << "Download request sent";
}

void ExtensionStoreUI::OnFetchExtensionsComplete(std::unique_ptr<std::string> response_body) {
  LOG(ERROR) << "OnFetchExtensionsComplete called";  // Add this to verify callback is reached
  
  if (!response_body) {
    LOG(ERROR) << "Failed to fetch extensions.json - response body is null";
    // Notify frontend of error
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to fetch extensions"));
    return;
  }

  if (response_body->empty()) {
    LOG(ERROR) << "Response body is empty";
    content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Empty response received"));
    return;
  }

  LOG(ERROR) << "Raw response received, length: " << response_body->length();
  LOG(ERROR) << "Response content: " << *response_body;

  // Parse the JSON response.
  absl::optional<base::Value> json = base::JSONReader::Read(*response_body);
  if (!json || !json->is_dict()) {
    LOG(ERROR) << "Failed to parse extensions.json";
    return;
  }

  LOG(ERROR) << "JSON parsed successfully";

  const base::Value::Dict& root_dict = json->GetDict();
  const base::Value::List* extensions = root_dict.FindList("extensions");
  if (!extensions) {
    LOG(ERROR) << "No extensions found in extensions.json";
    return;
  }

  LOG(ERROR) << "Found " << extensions->size() << " extensions";

  // Log each extension's details
  for (const base::Value& extension : *extensions) {
    if (!extension.is_dict()) {
      LOG(ERROR) << "Invalid extension entry (not a dictionary)";
      continue;
    }

    const base::Value::Dict& extension_dict = extension.GetDict();
    LOG(ERROR) << "Processing extension: ";
    const std::string* icon_url = extension_dict.FindString("icon_url");
    const std::string* download_url = extension_dict.FindString("download_url");

    if (icon_url) {
      FetchIcon(*icon_url, extension_dict.Clone());
    }

    if (download_url) {
      FetchDownloadUrl(*download_url, extension_dict.Clone());
    }
  }
}

void ExtensionStoreUI::FetchIcon(const std::string& icon_url, base::Value::Dict extension_dict) {
  LOG(ERROR) << "Starting icon fetch for URL: " << icon_url;
  
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(icon_url);
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation(
      "extension_store_fetch_icon", R"(
        semantics {
          sender: "Extension Store"
          description: "Fetching extension icon from GitHub."
          trigger: "User opens the extension store."
          data: "None."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

  auto loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

  // Get the BrowserContext from the WebUI.
  content::BrowserContext* browser_context =
      content::WebUIController::web_ui()->GetWebContents()->GetBrowserContext();

  loader->DownloadToString(
      browser_context->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess()
          .get(),
      base::BindOnce(&ExtensionStoreUI::OnFetchIconComplete,
                     weak_factory_.GetWeakPtr(),
                     std::move(extension_dict)),
      1024 * 1024);
}

void ExtensionStoreUI::OnFetchIconComplete(base::Value::Dict extension_dict,
                                           std::unique_ptr<std::string> response_body) {
  if (!response_body) {
    LOG(ERROR) << "Failed to fetch icon";
    return;
  }

  LOG(ERROR) << "Icon fetch completed successfully, size: " << response_body->size() << " bytes";

  // Convert the response body to a span<const uint8_t>.
  base::span<const uint8_t> input_span(
      reinterpret_cast<const uint8_t*>(response_body->data()),
      response_body->size());

  // Encode the icon as base64.
  std::string base64_icon;
  base::Base64EncodeAppend(input_span, &base64_icon);
  extension_dict.Set("icon_base64", base64_icon);

  // Send the updated extension data to the frontend.
  base::Value response(std::move(extension_dict));
  LOG(ERROR) << "Sending extension data to frontend after icon fetch";

  content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe("handleExtensionData", response);
}

void ExtensionStoreUI::FetchDownloadUrl(const std::string& download_url, base::Value::Dict extension_dict) {
  LOG(ERROR) << "Starting download URL fetch for: " << download_url;
  
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(download_url);
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation(
      "extension_store_fetch_download_url", R"(
        semantics {
          sender: "Extension Store"
          description: "Fetching extension download URL from GitHub."
          trigger: "User opens the extension store."
          data: "None."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled by settings."
          policy_exception_justification: "Not implemented."
        })");

  auto loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

  // Get the BrowserContext from the WebUI.
  content::BrowserContext* browser_context =
      content::WebUIController::web_ui()->GetWebContents()->GetBrowserContext();

  loader->DownloadToString(
      browser_context->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess()
          .get(),
      base::BindOnce(&ExtensionStoreUI::OnFetchDownloadUrlComplete,
                     weak_factory_.GetWeakPtr(),
                     std::move(extension_dict)),
      1024 * 1024);
}

void ExtensionStoreUI::OnFetchDownloadUrlComplete(base::Value::Dict extension_dict,
                                                  std::unique_ptr<std::string> response_body) {
  if (!response_body) {
    LOG(ERROR) << "Failed to fetch download URL";
    return;
  }

  LOG(ERROR) << "Download URL fetch completed: " << *response_body;

  // Update the extension data with the download URL.
  extension_dict.Set("download_url", *response_body);

  // Send the updated extension data to the frontend.
  base::Value response(std::move(extension_dict));
  LOG(ERROR) << "Sending extension data to frontend after download URL fetch";

  content::WebUIController::web_ui()->CallJavascriptFunctionUnsafe("handleExtensionData", response);
}