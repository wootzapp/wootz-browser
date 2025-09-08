#include "chrome/browser/ui/webui/extension_store/extension_store_ui.h"
#include "chrome/browser/ui/webui/extension_store/extension_store_constants.h"

#include <memory>
#include <string>
#include <utility>

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/extension_store_resources.h"
#include "chrome/grit/extension_store_resources_map.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "net/base/load_flags.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "chrome/browser/history/history_service_factory.h"
#include "components/history/core/browser/history_service.h"
#include "components/history/core/browser/history_types.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"

namespace {

void CreateAndAddExtensionStoreHTMLSource(Profile* profile) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIExtensionStoreHost);
  
  // Make sure all resources are properly registered
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kExtensionStoreResources, kExtensionStoreResourcesSize),
      IDR_EXTENSION_STORE_EXTENSION_STORE_HTML);
      
  
  // Override CSP to allow external resources if needed
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc,
      "connect-src 'self' https://raw.githubusercontent.com;");
      
  // Add default strings or other resources if needed
  source->AddString("loadingMessage", "Loading extensions...");
}

}  // namespace

// ExtensionStoreUI Implementation
ExtensionStoreUI::ExtensionStoreUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  
  // Add the message handler
  web_ui->AddMessageHandler(std::make_unique<ExtensionStoreMessageHandler>(web_ui));
  
  // Set up the extension store HTML source
  CreateAndAddExtensionStoreHTMLSource(Profile::FromWebUI(web_ui));
}

ExtensionStoreUI::~ExtensionStoreUI() {
  // The WebUI framework will handle cleanup of message handlers
}

// ExtensionStoreMessageHandler Implementation
ExtensionStoreMessageHandler::ExtensionStoreMessageHandler(content::WebUI* web_ui)
    : is_destroyed_(false),
      web_ui_(web_ui),
      weak_factory_(this) {
  Profile* profile = Profile::FromWebUI(web_ui_);
    history::HistoryService* history_service =
        HistoryServiceFactory::GetForProfile(profile, ServiceAccessType::EXPLICIT_ACCESS);
    if (history_service) {
        history_service->AddObserver(this);
    }
}

ExtensionStoreMessageHandler::~ExtensionStoreMessageHandler() {
  // Set the destroyed flag first to prevent any new callbacks from proceeding
  is_destroyed_ = true;
  
  // Clean up resources
  CleanupResources();
}

void ExtensionStoreMessageHandler::CleanupResources() {
  // Cancel any ongoing URL loads
  if (extensions_loader_) {
    extensions_loader_.reset();
  }
  
  // Clear all icon loaders
  if (!icon_loaders_.empty()) {
    icon_loaders_.clear();
  }
  
  // Clear any stored data
  extensions_data_.clear();
  Profile* profile = Profile::FromWebUI(web_ui_);
    history::HistoryService* history_service =
        HistoryServiceFactory::GetForProfile(profile, ServiceAccessType::EXPLICIT_ACCESS);
    if (history_service) {
        history_service->RemoveObserver(this);
    }
}

void ExtensionStoreMessageHandler::OnJavascriptDisallowed() {
  weak_factory_.InvalidateWeakPtrs();
}

void ExtensionStoreMessageHandler::RegisterMessages() {
  // Register the message callback for fetching extensions
  web_ui_->RegisterMessageCallback(
      "fetchExtensions",
      base::BindRepeating(&ExtensionStoreMessageHandler::HandleFetchExtensions,
                         base::Unretained(this)));
  
  // Register the message callback for fetching icons
  web_ui_->RegisterMessageCallback(
      "fetchIcon",
      base::BindRepeating(&ExtensionStoreMessageHandler::HandleFetchIcon,
                         base::Unretained(this)));
                         
  // Register the message callback for fetching installed extensions
  web_ui_->RegisterMessageCallback(
      "fetchInstalledExtensions",
      base::BindRepeating(&ExtensionStoreMessageHandler::HandleFetchInstalledExtensions,
                         base::Unretained(this)));
}

void ExtensionStoreMessageHandler::HandleFetchExtensions(const base::Value::List& args) {
  if (is_destroyed_) {
    return;
  }
  
  auto resource_request = std::make_unique<network::ResourceRequest>();
  std::string url = std::string(extension_store::kExtensionStoreBaseUrl) + 
                    "?nocache=" + base::NumberToString(base::Time::Now().ToInternalValue());
  GURL fetch_url(url);
  
  if (!fetch_url.is_valid()) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Invalid URL"));
    return;
  }
  
  // Verify URL scheme
  if (!fetch_url.SchemeIs("https")) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Invalid URL scheme"));
    return;
  }
  
  resource_request->url = fetch_url;
  
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  
  // Add additional headers to help with debugging
  resource_request->headers.SetHeader("Accept", "application/json");
  resource_request->headers.SetHeader("User-Agent", "Chrome Extension Store");

  // To bypass the cache loading
  // Note: LOAD_BYPASS_CACHE is used to ensure we always get the latest data
  resource_request->load_flags = net::LOAD_BYPASS_CACHE;
  
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
        // Response started callback
      }));

  // Add download progress callback
  loader->SetOnDownloadProgressCallback(base::BindRepeating(
      [](uint64_t current) {
        // Download progress callback
      }));

  content::BrowserContext* browser_context =
      web_ui_->GetWebContents()->GetBrowserContext();
  
  if (!browser_context) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Browser context error"));
    return;
  }

  auto url_loader_factory = browser_context->GetDefaultStoragePartition()
                               ->GetURLLoaderFactoryForBrowserProcess();
  
  if (!url_loader_factory) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("URL loader factory error"));
    return;
  }
  
  // Save the loader as a class member to keep it alive
  extensions_loader_ = std::move(loader);
  
  extensions_loader_->DownloadToString(
      url_loader_factory.get(),
      base::BindOnce(&ExtensionStoreMessageHandler::OnFetchExtensionsComplete,
                     weak_factory_.GetWeakPtr()),
      1024 * 1024);  // 1MB max size
}

void ExtensionStoreMessageHandler::OnFetchExtensionsComplete(std::unique_ptr<std::string> response_body) {
  if (!response_body) {
    // Notify frontend of error
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to fetch extensions"));
    return;
  }

  if (response_body->empty()) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Empty response received"));
    return;
  }

  // Parse the JSON response.
  std::optional<base::Value> json = base::JSONReader::Read(*response_body);
  if (!json || !json->is_dict()) {
    return;
  }

  const base::Value::Dict& root_dict = json->GetDict();
  const base::Value::List* extensions = root_dict.FindList("extensions");
  if (!extensions) {
    return;
  }
  
  // Clear existing extensions data
  extensions_data_.clear();

  // Process each extension
  for (const base::Value& extension_value : *extensions) {
    if (!extension_value.is_dict()) {
      continue;
    }

    const base::Value::Dict& extension_dict = extension_value.GetDict();
    
    // Create a copy of the extension data for storage
    base::Value::Dict extension_copy = extension_dict.Clone();
    
    // Extract required fields
    const std::string* id = extension_dict.FindString("id");
    const std::string* name = extension_dict.FindString("name");
    const std::string* description = extension_dict.FindString("description");
    const std::string* version = extension_dict.FindString("version");
    const std::string* icon_url = extension_dict.FindString("icon_url");
    
    if (!id || !name || !description || !version) {
      continue;
    }
    
    // Store the extension data with the ID as key
    extensions_data_[*id] = std::move(extension_copy);
    
    // Send the extension data to the frontend immediately
    SendExtensionToFrontend(*id);
    
    // Fetch the icon if available
    if (icon_url && !icon_url->empty()) {
      FetchIcon(*icon_url, *id);
    }
  }
}

void ExtensionStoreMessageHandler::SendExtensionToFrontend(const std::string& extension_id) {
  auto it = extensions_data_.find(extension_id);
  if (it == extensions_data_.end()) {
    return;
  }
  
  // Create a copy of the extension data to send
  base::Value extension_value(it->second.Clone());
  
  // Send the extension data to the frontend
  web_ui_->CallJavascriptFunctionUnsafe(
      "handleExtensionData", extension_value);
}

void ExtensionStoreMessageHandler::FetchIcon(const std::string& icon_url, const std::string& extension_id) {
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
      web_ui_->GetWebContents()->GetBrowserContext();

  // Store the loader in the map with extension_id as the key
  std::string loader_key = extension_id;
  auto* loader_ptr = loader.get();
  icon_loaders_[loader_key] = std::move(loader);
  
  loader_ptr->DownloadToString(
      browser_context->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess()
          .get(),
      base::BindOnce(&ExtensionStoreMessageHandler::OnFetchIconComplete,
                    weak_factory_.GetWeakPtr(),
                    extension_id, // Pass extension_id to the callback
                    loader_key),  // Pass the key to identify the loader
      1024 * 1024);  // 1MB max size
}

void ExtensionStoreMessageHandler::OnFetchIconComplete(
    const std::string& extension_id,
    const std::string& loader_key,
    std::unique_ptr<std::string> response_body) {
  // Remove the loader from the map since it's done
  auto it = icon_loaders_.find(loader_key);
  if (it != icon_loaders_.end()) {
    icon_loaders_.erase(it);
  }

  if (!response_body) {
    return;
  }

  // Check if we have the extension data
  auto ext_it = extensions_data_.find(extension_id);
  if (ext_it == extensions_data_.end()) {
    return;
  }

  // Convert the response body to a span<const uint8_t>.
  base::span<const uint8_t> input_span(
      reinterpret_cast<const uint8_t*>(response_body->data()),
      response_body->size());

  // Encode the icon as base64.
  std::string base64_icon;
  base::Base64EncodeAppend(input_span, &base64_icon);

  // Update the extension data with the base64 icon
  ext_it->second.Set("icon_base64", base64_icon);
  
  // Send the updated extension to the frontend
  SendExtensionToFrontend(extension_id);
}

void ExtensionStoreMessageHandler::HandleFetchIcon(const base::Value::List& args) {
  if (args.size() < 1 || !args[0].is_string()) {
    return;
  }

  std::string icon_url = args[0].GetString();
  
  // Extract the extension ID from the second parameter if available
  std::string extension_id;
  if (args.size() >= 2 && args[1].is_string()) {
    extension_id = args[1].GetString();
  } else {
    return;
  }

  // Call the FetchIcon method with both the icon_url and extension_id
  FetchIcon(icon_url, extension_id);
}

void ExtensionStoreMessageHandler::HandleFetchInstalledExtensions(const base::Value::List& args) {
  if (is_destroyed_) {
    return;
  }
  
  LOG(INFO) << " HandleFetchInstalledExtensions called";
  
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    LOG(ERROR) << " Failed to get profile in HandleFetchInstalledExtensions";
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to get profile"));
    return;
  }
  
  LOG(INFO) << " Successfully retrieved profile";
  
  // Get the extension registry
  extensions::ExtensionRegistry* registry = extensions::ExtensionRegistry::Get(profile);
  if (!registry) {
    LOG(ERROR) << " Failed to get extension registry";
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to get extension registry"));
    return;
  }
  
  LOG(INFO) << " Successfully retrieved extension registry";
  
  // Create a list to hold installed extension info
  base::Value::List installed_extensions_list;
  
  // Get the enabled (installed) extensions
  const extensions::ExtensionSet& installed_extensions = registry->enabled_extensions();
  
  LOG(INFO) << " Number of installed extensions found: " << installed_extensions.size();
  
  // Iterate through installed extensions and add their info to the list
  for (const auto& extension : installed_extensions) {
    base::Value::Dict extension_info;
    
    extension_info.Set("id", extension->id());
    extension_info.Set("name", extension->name());
    extension_info.Set("version", extension->version().GetString());
    extension_info.Set("description", extension->description());
    
    LOG(INFO) << " Adding installed extension: " << extension->id() << " - " << extension->name();
    
    installed_extensions_list.Append(std::move(extension_info));
  }
  
  LOG(INFO) << " Sending " << installed_extensions_list.size() << " installed extensions to frontend";
  
  // Send the installed extensions info to the frontend
  web_ui_->CallJavascriptFunctionUnsafe(
      "handleInstalledExtensionsData", base::Value(std::move(installed_extensions_list)));
      
  LOG(INFO) << " Completed sending installed extensions data to frontend";
}