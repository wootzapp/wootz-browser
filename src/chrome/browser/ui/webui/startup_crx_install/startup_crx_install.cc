#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install.h"

#include <algorithm>
#include <optional>
#include <string>
#include <cctype>
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "chrome/grit/startup_crx_install_resources.h"
#include "chrome/grit/startup_crx_install_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"
#include "base/values.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "content/public/browser/storage_partition.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "base/base64.h"
#include "base/logging.h"
#include "base/functional/bind.h"
#include "base/task/sequenced_task_runner.h"

// Add this to access shared preferences
#include "components/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"

// Add this include at the top
#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install_prefs.h"
#include "chrome/browser/ui/webui/extension_store/extension_store_constants.h"

// StartupCrxInstallMessageHandler implementation
StartupCrxInstallMessageHandler::StartupCrxInstallMessageHandler(content::WebUI* web_ui)
    : web_ui_(web_ui),
      weak_factory_(this) {
  LOG(INFO) << "StartupCrxInstallMessageHandler constructor called";
  
  // Don't fetch extensions data here - wait for JavaScript to be ready
}
StartupCrxInstallMessageHandler::~StartupCrxInstallMessageHandler() {
  is_destroyed_ = true;
}
void StartupCrxInstallMessageHandler::OnJavascriptDisallowed() {
  weak_factory_.InvalidateWeakPtrs();
}

void StartupCrxInstallMessageHandler::RegisterMessages() {
  // Register message handlers
  LOG(INFO) << "Registering message handlers";
  
  web_ui_->RegisterMessageCallback(
      "fetchInstalledExtensions",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleFetchInstalledExtensions,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered fetchInstalledExtensions handler";

  web_ui_->RegisterMessageCallback(
      "downloadArtifactExtension",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleDownloadExtension,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered downloadArtifactExtension handler";

  web_ui_->RegisterMessageCallback(
      "getUtmSource",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleGetUtmSource,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered getUtmSource handler";
  
  web_ui_->RegisterMessageCallback(
      "getExtensionData",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleGetExtensionData,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered getExtensionData handler";

  web_ui_->RegisterMessageCallback(
      "installDefaultExtensions",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleInstallDefaultExtensions,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered installDefaultExtensions handler";
  
  web_ui_->RegisterMessageCallback(
      "onExtensionInstallComplete",
      base::BindRepeating(&StartupCrxInstallMessageHandler::OnExtensionInstallComplete,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered onExtensionInstallComplete handler";
  
  // Add a test message that we can trigger manually
  web_ui_->RegisterMessageCallback(
      "testMessageHandler",
      base::BindRepeating([](const base::Value::List& args) {
        LOG(INFO) << "Test message handler called with " << args.size() << " arguments";
      }));
  LOG(INFO) << "Registered testMessageHandler";
}

// Handles the message for installing default extensions.
void StartupCrxInstallMessageHandler::HandleInstallDefaultExtensions(const base::Value::List& args) {
  LOG(INFO) << "HandleInstallDefaultExtensions called";
  if (is_destroyed_) return;
  FetchExtensionsDataForDefaultInstall();
}

// Fetches extensions.json and filters for default_extension = true
void StartupCrxInstallMessageHandler::FetchExtensionsDataForDefaultInstall() {
  LOG(INFO) << "Fetching extensions.json for default install";

  auto resource_request = std::make_unique<network::ResourceRequest>();

  std::string fetch_url = std::string(extension_store::kExtensionStoreBaseUrl);

  resource_request->url = GURL(fetch_url);
  resource_request->method = "GET";

  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("startup_crx_install_default_extensions", R"(
        semantics {
          sender: "Startup CRX Install"
          description: "Fetches extensions data for default install."
          trigger: "First run extension install."
          data: "No user data sent."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled."
          policy_exception_justification: "Not implemented."
        })");
  Profile* profile = Profile::FromWebUI(web_ui_);
  auto* storage_partition = profile->GetDefaultStoragePartition();
  extensions_loader_ = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);
  extensions_loader_->DownloadToString(
      storage_partition->GetURLLoaderFactoryForBrowserProcess().get(),
      base::BindOnce(&StartupCrxInstallMessageHandler::OnDefaultExtensionsDataFetched,
                     weak_factory_.GetWeakPtr()),
      1024 * 1024 /* 1MB max */);
}

// Callback for when default extensions data is fetched
void StartupCrxInstallMessageHandler::OnDefaultExtensionsDataFetched(std::optional<std::string> response_body) {
  LOG(INFO) << "OnDefaultExtensionsDataFetched called";
  extensions_loader_.reset();
  if (!response_body || response_body->empty()) {
    LOG(ERROR) << "Failed to fetch default extensions data: no response body or empty response";
    return;
  }
  auto json = base::JSONReader::Read(*response_body, base::JSON_REPLACE_INVALID_CHARACTERS);
  if (!json || !json->is_dict()) {
    LOG(ERROR) << "Invalid extensions.json format.";
    return;
  }
  const base::Value::Dict& root_dict = json->GetDict();
  const base::Value::List* ext_list = root_dict.FindList("extensions");
  if (!ext_list) {
    LOG(ERROR) << "No 'extensions' array found in extensions JSON";
    return;
  }
  LOG(INFO) << "Total extensions in JSON: " << ext_list->size();
  
  // Collect all default extensions first
  default_extensions_queue_.clear();
  for (const auto& ext_val : *ext_list) {
    if (!ext_val.is_dict()) continue;
    const base::Value::Dict& ext_dict = ext_val.GetDict();
    
    // Log each extension's default_extension status
    bool is_default = ext_dict.FindBool("default_extension").value_or(false);
    const std::string* name_ptr = ext_dict.FindString("name");
    std::string name = name_ptr ? *name_ptr : "Unknown";
    LOG(INFO) << "Extension: " << name << ", default_extension: " << (is_default ? "true" : "false");
    
    if (is_default) {
      const std::string* id_ptr = ext_dict.FindString("id");
      const std::string* download_url_ptr = ext_dict.FindString("download_url");
      const std::string* name_ptr = ext_dict.FindString("name");
      const std::string* description_ptr = ext_dict.FindString("description");
      const std::string* version_ptr = ext_dict.FindString("version");
      const std::string* icon_url_ptr = ext_dict.FindString("icon_url");
      
      if (id_ptr && download_url_ptr && name_ptr) {
        DefaultExtensionInfo ext_info;
        ext_info.id = *id_ptr;
        ext_info.download_url = *download_url_ptr;
        ext_info.name = *name_ptr;
        ext_info.description = description_ptr ? *description_ptr : "";
        ext_info.version = version_ptr ? *version_ptr : "";
        ext_info.icon_url = icon_url_ptr ? *icon_url_ptr : "";
        
        default_extensions_queue_.push_back(ext_info);
        LOG(INFO) << "Added default extension to queue: " << ext_info.name << " (" << ext_info.id << ")";
      }
    }
  }
  
  LOG(INFO) << "Found " << default_extensions_queue_.size() << " default extensions to install";
  
  // Start installing extensions serially
  current_extension_index_ = 0;
  InstallNextDefaultExtension();
}

// Installs the next default extension in the queue
void StartupCrxInstallMessageHandler::InstallNextDefaultExtension() {
  LOG(INFO) << "InstallNextDefaultExtension called, index: " << current_extension_index_ 
            << ", queue size: " << default_extensions_queue_.size();
  
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  // Check if we've processed all extensions
  if (current_extension_index_ >= default_extensions_queue_.size()) {
    LOG(INFO) << "All default extensions have been processed";
    // Notify JavaScript that all default extensions have been processed
    web_ui_->CallJavascriptFunctionUnsafe("handleDefaultExtensionsComplete");
    return;
  }
  
  const DefaultExtensionInfo& current_ext = default_extensions_queue_[current_extension_index_];
  LOG(INFO) << "Installing default extension " << (current_extension_index_ + 1) 
            << " of " << default_extensions_queue_.size() << ": " << current_ext.name;
  
  // Send progress update to frontend
  base::Value::Dict progress;
  progress.Set("currentIndex", static_cast<int>(current_extension_index_));
  progress.Set("totalCount", static_cast<int>(default_extensions_queue_.size()));
  progress.Set("extensionName", current_ext.name);
  progress.Set("extensionId", current_ext.id);
  progress.Set("state", "installing");
  
  web_ui_->CallJavascriptFunctionUnsafe("handleDefaultExtensionProgress", base::Value(std::move(progress)));
  
  // Install the current extension
  base::Value::List install_args;
  install_args.Append(current_ext.download_url);
  install_args.Append(current_ext.id);
  install_args.Append(current_ext.name);
  install_args.Append(current_ext.description);
  install_args.Append(current_ext.version);
  install_args.Append(current_ext.icon_url);
  
  this->HandleDownloadExtension(install_args);
  
  // Move to next extension after a delay to allow current one to install
  // The completion callback will handle moving to the next extension
  current_extension_index_++;
  auto timer = base::Seconds(5); // Fallback timer in case completion callback doesn't fire
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&StartupCrxInstallMessageHandler::InstallNextDefaultExtension,
                     weak_factory_.GetWeakPtr()),
      timer);
}

// Callback when an extension installation is complete
void StartupCrxInstallMessageHandler::OnExtensionInstallComplete(const base::Value::List& args) {
  LOG(INFO) << "OnExtensionInstallComplete called";
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  // This method is called from JavaScript when an extension installation is complete
  LOG(INFO) << "Extension installation completed, moving to next extension";

  auto delay = base::Milliseconds(1000); // Short delay of 1 second before next installation, assuming the extension is sent for installation
  // Continue with the next extension installation after a short delay
  // Note: current_extension_index_ is already incremented in InstallNextDefaultExtension
  base::SequencedTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&StartupCrxInstallMessageHandler::InstallNextDefaultExtension,
                     weak_factory_.GetWeakPtr()),
      delay);
}


void StartupCrxInstallMessageHandler::HandleGetUtmSource(const base::Value::List& args) {
  LOG(INFO) << "HandleGetUtmSource called with " << args.size() << " arguments";
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  LOG(INFO) << "Handling getUtmSource";
  
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    LOG(ERROR) << "Failed to get profile in HandleGetUtmSource";
    SendUtmToFrontend("");
    return;
  }
  LOG(INFO) << "Profile obtained successfully";
  
  // Get UTM source from preferences
  PrefService* prefs = profile->GetPrefs();
  if (!prefs) {
    LOG(ERROR) << "Failed to get prefs in HandleGetUtmSource";
    SendUtmToFrontend("");
    return;
  }
  LOG(INFO) << "PrefService obtained successfully";
  
  std::string utm_source;
  
  // Check if the preference exists before getting it
  LOG(INFO) << "Looking for preference: " << startup_crx_install::kUtmSourcePref;
  const PrefService::Preference* pref = 
      prefs->FindPreference(startup_crx_install::kUtmSourcePref);
  if (pref) {
    LOG(INFO) << "Preference found: " << startup_crx_install::kUtmSourcePref;
    utm_source = prefs->GetString(startup_crx_install::kUtmSourcePref);
    LOG(INFO) << "Retrieved UTM source value: " << utm_source;
  } else {
    LOG(ERROR) << "Preference not found: " << startup_crx_install::kUtmSourcePref;
    // Try the non-namespaced preference as fallback
    pref = prefs->FindPreference("utm_source");
    if (pref) {
      LOG(INFO) << "Found non-namespaced preference: utm_source";
      utm_source = prefs->GetString("utm_source");
      LOG(INFO) << "Retrieved UTM source value from non-namespaced pref: " << utm_source;
    } else {
      LOG(ERROR) << "Neither preference was found";
    }
  }
  LOG(ERROR) << "Final UTM source: " << utm_source;
  SendUtmToFrontend(utm_source);
}

void StartupCrxInstallMessageHandler::SendUtmToFrontend(const std::string& utm_source) {
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  LOG(INFO) << "Sending UTM source to frontend: " << utm_source;
  
  // Create a base::Value for the UTM source
  base::Value utm_value(utm_source);
  
  std::string debug_json;
  base::JSONWriter::Write(utm_value, &debug_json);
  LOG(INFO) << "UTM value as JSON: " << debug_json;
  
  // Call the JavaScript function
  web_ui_->CallJavascriptFunctionUnsafe("handleUtmSource", utm_value);
  
  LOG(INFO) << "Sent UTM source to JS: " << utm_source;
}

void StartupCrxInstallMessageHandler::HandleGetExtensionData(const base::Value::List& args) {
  LOG(INFO) << "HandleGetExtensionData called";
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  // Fetch extensions data when JavaScript requests it
  FetchExtensionsData();
}

void StartupCrxInstallMessageHandler::HandleFetchInstalledExtensions(const base::Value::List& args) {
  if (is_destroyed_) {
    return;
  }
  LOG(INFO) << "Handling fetchInstalledExtensions";
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to get profile"));
    return;
  }
  
  // Get the extension registry
  LOG(INFO) << "Getting extension registry";
  extensions::ExtensionRegistry* registry = extensions::ExtensionRegistry::Get(profile);
  if (!registry) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Failed to get extension registry"));
    return;
  }
  
  // Create a list to hold installed extension info
  base::Value::List installed_extensions_list;
  
  // Get the enabled (installed) extensions
  const extensions::ExtensionSet& installed_extensions = registry->enabled_extensions();
  
  // Iterate through installed extensions and add their info to the list
  LOG(INFO) << "Iterating through installed extensions";
  for (const auto& extension : installed_extensions) {
    base::Value::Dict extension_info;
    
    extension_info.Set("id", extension->id());
    extension_info.Set("name", extension->name());
    extension_info.Set("version", extension->version().GetString());
    extension_info.Set("description", extension->description());
    
    installed_extensions_list.Append(std::move(extension_info));
  }
  
  // Send the installed extensions info to the frontend
  LOG(INFO) << "Sending installed extensions info to frontend";
  web_ui_->CallJavascriptFunctionUnsafe(
      "handleInstalledExtensionsData", base::Value(std::move(installed_extensions_list)));
  LOG(INFO) << "Sent installed extensions info to frontend";
}

void StartupCrxInstallMessageHandler::HandleDownloadExtension(const base::Value::List& args) {
  LOG(INFO) << "Handling downloadArtifactExtension";
  if (is_destroyed_) {
    return;
  }
  
  LOG(INFO) << "Checking if download URL is valid";
  if (args.empty() || !args[0].is_string()) {
    web_ui_->CallJavascriptFunctionUnsafe(
        "handleError", base::Value("Invalid download URL"));
    return;
  }

  LOG(INFO) << "Getting download URL";
  std::string download_url = args[0].GetString();
  std::string extension_id = args.size() > 1 && args[1].is_string() ? args[1].GetString() : "";
  std::string extension_name = args.size() > 2 && args[2].is_string() ? args[2].GetString() : "";
  std::string extension_description = args.size() > 3 && args[3].is_string() ? args[3].GetString() : "";
  std::string extension_version = args.size() > 4 && args[4].is_string() ? args[4].GetString() : "";
  std::string extension_icon_url = args.size() > 5 && args[5].is_string() ? args[5].GetString() : "";
  
  LOG(INFO) << "Installing extension programmatically: " << extension_name << " (" << extension_id << ") from: " << download_url;
  
  // Send progress updates to the frontend
  base::Value::Dict progress;
  progress.Set("state", "installing");
  progress.Set("percentComplete", 0);
  progress.Set("extensionName", extension_name);
  progress.Set("extensionId", extension_id);
  LOG(INFO) << "Sending progress updates to frontend";
  web_ui_->CallJavascriptFunctionUnsafe(
      "handleDownloadProgress", base::Value(std::move(progress)));
  LOG(INFO) << "Sent progress updates to frontend";
  
  // Use programmatic extension installation instead of direct download
  // This avoids the harmful download popup by using Chrome's extension installation API
  base::Value::Dict extension_data;
  extension_data.Set("download_url", download_url);
  extension_data.Set("id", extension_id);
  extension_data.Set("name", extension_name);
  extension_data.Set("description", extension_description);
  extension_data.Set("version", extension_version);
  extension_data.Set("icon_url", extension_icon_url);
  extension_data.Set("install_method", "programmatic"); // Flag to indicate programmatic install
  
  web_ui_->CallJavascriptFunctionUnsafe("InstallExtensionProgrammatically", base::Value(std::move(extension_data)));
  LOG(INFO) << "Programmatic extension installation initiated for: " << extension_name;
}

void StartupCrxInstallMessageHandler::FetchExtensionsData() {
  LOG(INFO) << "FetchExtensionsData called";
  
  auto request = std::make_unique<network::ResourceRequest>();

  std::string fetch_url = std::string(extension_store::kExtensionStoreBaseUrl);

  request->url = GURL(fetch_url);
  request->method = "GET";
  
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("startup_crx_install_extensions_fetch", R"(
        semantics {
          sender: "Startup CRX Install"
          description: "Fetches extensions data from GitHub repository."
          trigger: "User visits startup CRX install page."
          data: "No user data sent."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled."
          policy_exception_justification: "Not implemented."
        })");
  
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    LOG(ERROR) << "Failed to get profile for extensions data fetch";
    return;
  }
  
  auto url_loader_factory = profile->GetDefaultStoragePartition()
                               ->GetURLLoaderFactoryForBrowserProcess();
  
  extensions_loader_ = network::SimpleURLLoader::Create(std::move(request), traffic_annotation);
  
  extensions_loader_->DownloadToString(
      url_loader_factory.get(),
      base::BindOnce(&StartupCrxInstallMessageHandler::OnExtensionsDataFetched,
                     weak_factory_.GetWeakPtr()),
                    1024*1024);
  
  LOG(INFO) << "Extensions data fetch request sent";
}

void StartupCrxInstallMessageHandler::OnExtensionsDataFetched(
    std::optional<std::string> response_body) {
  
  LOG(INFO) << "OnExtensionsDataFetched called";
  
  // Clear the loader since the request is complete
  extensions_loader_.reset();
  
  if (!response_body || response_body->empty()) {
    LOG(ERROR) << "Failed to fetch extensions data: no response body or empty response";
    return;
  }
  
  LOG(INFO) << "Extensions data fetched successfully, size: " << response_body->size();
  
  // Log the raw JSON data
  LOG(INFO) << "Raw JSON data: " << *response_body;
  
  // Get the current UTM source to compare
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    LOG(ERROR) << "Failed to get profile for UTM source";
    return;
  }
  
  PrefService* prefs = profile->GetPrefs();
  if (!prefs) {
    LOG(ERROR) << "Failed to get prefs for UTM source";
    return;
  }
  
  std::string utm_source;
  const PrefService::Preference* pref = 
      prefs->FindPreference(startup_crx_install::kUtmSourcePref);
  if (pref) {
    utm_source = prefs->GetString(startup_crx_install::kUtmSourcePref);
  } else {
    // Try the non-namespaced preference as fallback
    pref = prefs->FindPreference("utm_source");
    if (pref) {
      utm_source = prefs->GetString("utm_source");
    }
  }
  
  LOG(INFO) << "Current UTM source: " << utm_source;
  
  // Parse and log the extension data
  ParseAndLogExtensionData(*response_body, utm_source);
}

void StartupCrxInstallMessageHandler::HandleExtensionsDataFetchError(const std::string& error_message) {
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  LOG(ERROR) << "Extensions data fetch failed: " << error_message;
  
  // Notify the frontend about the error
  base::Value::Dict error_info;
  error_info.Set("type", "network_error");
  error_info.Set("message", error_message);
  error_info.Set("timestamp", static_cast<double>(base::Time::Now().InMillisecondsSinceUnixEpoch()));
  
  web_ui_->CallJavascriptFunctionUnsafe("handleExtensionsFetchError", base::Value(std::move(error_info)));
  
  // Try to get cached extension data or fallback to a minimal UI
  Profile* profile = Profile::FromWebUI(web_ui_);
  if (profile) {
    PrefService* prefs = profile->GetPrefs();
    if (prefs) {
      std::string utm_source;
      const PrefService::Preference* pref = 
          prefs->FindPreference(startup_crx_install::kUtmSourcePref);
      if (pref) {
        utm_source = prefs->GetString(startup_crx_install::kUtmSourcePref);
      } else {
        pref = prefs->FindPreference("utm_source");
        if (pref) {
          utm_source = prefs->GetString("utm_source");
        }
      }
      
      if (!utm_source.empty()) {
        LOG(INFO) << "Providing fallback extension data for UTM source: " << utm_source;
        // Provide basic fallback extension data
        ProvideFallbackExtensionData(utm_source);
      }
    }
  }
}

void StartupCrxInstallMessageHandler::ProvideFallbackExtensionData(const std::string& utm_source) {
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  LOG(INFO) << "Providing fallback extension data for UTM source: " << utm_source;
  
  // Create fallback extension data based on known UTM sources
  std::string fallback_name;
  std::string fallback_id;
  std::string fallback_description;
  std::string fallback_version = "1.0.0";
  std::string fallback_download_url;
  
  // Convert UTM source to lowercase for comparison
  std::string utm_lower = utm_source;
  std::transform(utm_lower.begin(), utm_lower.end(), utm_lower.begin(), ::tolower);
  
  // Send fallback data to frontend with empty icon and indicate it's a fallback
  base::Value::Dict extension_data;
  extension_data.Set("name", fallback_name);
  extension_data.Set("icon_base64", ""); // No icon for fallback
  extension_data.Set("download_url", fallback_download_url);
  extension_data.Set("id", fallback_id);
  extension_data.Set("description", fallback_description);
  extension_data.Set("version", fallback_version);
  extension_data.Set("is_fallback", true); // Mark as fallback data
  extension_data.Set("fallback_reason", "Network request failed");
  
  // Send the fallback extension data to JavaScript
  web_ui_->CallJavascriptFunctionUnsafe("handleExtensionData", base::Value(std::move(extension_data)));
  
  LOG(INFO) << "Sent fallback extension data to frontend for: " << fallback_name;
}

void StartupCrxInstallMessageHandler::ParseAndLogExtensionData(
    const std::string& json_data, const std::string& utm_source) {
  
  LOG(INFO) << "ParseAndLogExtensionData called with UTM source: " << utm_source;
  
  // Check if this is a default extension installation (no UTM source needed)
  bool is_default_install = utm_source.empty();
  if (is_default_install) {
    LOG(INFO) << "UTM source is empty, this is a default extension installation";
  }
  
  auto parsed_json = base::JSONReader::Read(json_data);
  if (!parsed_json || !parsed_json->is_dict()) {
    LOG(ERROR) << "Failed to parse extensions JSON data";
    return;
  }
  
  LOG(INFO) << "Successfully parsed JSON data";
  
  const base::Value::Dict& root_dict = parsed_json->GetDict();
  
  // Log the keys available in the root dictionary
  LOG(INFO) << "Root dictionary keys:";
  for (const auto& [key, value] : root_dict) {
    LOG(INFO) << "  Key: " << key;
  }
  
  const base::Value::List* extensions_list = root_dict.FindList("extensions");
  
  if (!extensions_list) {
    LOG(ERROR) << "No 'extensions' array found in extensions JSON";
    return;
  }
  
  LOG(INFO) << "Found " << extensions_list->size() << " extensions in data";
  
  // For default extension installation, collect all default extensions first
  std::vector<DefaultExtensionInfo> default_extensions;
  
  // Iterate through each extension object
  for (const auto& item : *extensions_list) {
    if (!item.is_dict()) {
      LOG(WARNING) << "Skipping non-dictionary item in extensions array";
      continue;
    }
    
    const base::Value::Dict& extension_dict = item.GetDict();
    
    // For default extension installation, look for extensions with default_extension = true
    if (is_default_install) {
      bool is_default = extension_dict.FindBool("default_extension").value_or(false);
      if (!is_default) {
        continue; // Skip non-default extensions
      }
      
      LOG(INFO) << "Found default extension, collecting...";
      
      // Extract the required fields for default extension
      const std::string* icon_url = extension_dict.FindString("icon_url");
      const std::string* download_url = extension_dict.FindString("download_url");
      const std::string* id = extension_dict.FindString("id");
      const std::string* description = extension_dict.FindString("description");
      const std::string* version = extension_dict.FindString("version");
      const std::string* name = extension_dict.FindString("name");
      
      if (id && download_url && name) {
        DefaultExtensionInfo ext_info;
        ext_info.id = *id;
        ext_info.download_url = *download_url;
        ext_info.name = *name;
        ext_info.description = description ? *description : "";
        ext_info.version = version ? *version : "";
        ext_info.icon_url = icon_url ? *icon_url : "";
        
        default_extensions.push_back(ext_info);
        LOG(INFO) << "Added default extension to collection: " << ext_info.name << " (" << ext_info.id << ")";
      }
    } else {
      // For UTM-based installation, match by campaign
      const std::string* campaign = extension_dict.FindString("campaign");
      if (!campaign) {
        LOG(WARNING) << "Extension found without campaign field";
        continue;
      }
      
      LOG(INFO) << "Processing extension: " << *campaign;
      
      // Compare campaign with UTM source (case insensitive)
      std::string campaign_lower = *campaign;
      std::transform(campaign_lower.begin(), campaign_lower.end(), campaign_lower.begin(), ::tolower);
      
      std::string utm_lower = utm_source;
      std::transform(utm_lower.begin(), utm_lower.end(), utm_lower.begin(), ::tolower);

      LOG(INFO) << "Comparing '" << campaign_lower << "' with UTM source '" << utm_lower << "'";

      if (campaign_lower != utm_lower) {
        continue; // Skip non-matching extensions
      }
      
      LOG(INFO) << "Found matching extension for UTM source: " << utm_source;
      
      // Extract the required fields for UTM-based extension
      const std::string* icon_url = extension_dict.FindString("icon_url");
      const std::string* download_url = extension_dict.FindString("download_url");
      const std::string* id = extension_dict.FindString("id");
      const std::string* description = extension_dict.FindString("description");
      const std::string* version = extension_dict.FindString("version");
      const std::string* name = extension_dict.FindString("name");
      
      // Use the original extension ID from JSON data
      std::string final_extension_id = id ? *id : "";
      
      // Log all extracted parameters
      LOG(INFO) << "=== MATCHED EXTENSION DETAILS ===";
      LOG(INFO) << "Extension ID: " << final_extension_id;
      LOG(INFO) << "Extension Name: " << (name ? *name : "Not found");
      LOG(INFO) << "Extension Version: " << (version ? *version : "Not found");
      LOG(INFO) << "Extension Description: " << (description ? *description : "Not found");
      LOG(INFO) << "Icon URL: " << (icon_url ? *icon_url : "Not found");
      LOG(INFO) << "Download URL: " << (download_url ? *download_url : "Not found");
      LOG(INFO) << "=== END EXTENSION DETAILS ===";
      
      // Send the matched extension data to frontend, but first fetch the icon
      FetchIconImage(name ? *name : "", 
                     icon_url ? *icon_url : "", 
                     download_url ? *download_url : "",
                     final_extension_id,
                     description ? *description : "",
                     version ? *version : "");
      
      // For UTM-based installation, break after finding the first match
      break; // Found the matching extension, no need to continue
    }
  }
  
  // For default extension installation, process all collected extensions
  if (is_default_install && !default_extensions.empty()) {
    LOG(INFO) << "Processing " << default_extensions.size() << " default extensions";
    
    // Update the queue with all collected default extensions
    default_extensions_queue_ = std::move(default_extensions);
    current_extension_index_ = 0;
    
    // Start installing extensions serially
    InstallNextDefaultExtension();
  }
  
  LOG(INFO) << "Finished parsing extension data";
}

void StartupCrxInstallMessageHandler::FetchIconImage(
    const std::string& name,
    const std::string& icon_url,
    const std::string& download_url,
    const std::string& id,
    const std::string& description,
    const std::string& version) {
  
  LOG(INFO) << "FetchIconImage called for: " << name << " with icon URL: " << icon_url;
  
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  // If no icon URL, send with empty icon data
  if (icon_url.empty()) {
    LOG(INFO) << "No icon URL provided, sending extension data without icon";
    SendExtensionDataToFrontend(name, "", download_url, id, description, version);
    return;
  }
  
  // Create a network request for the icon
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(icon_url);
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("startup_crx_install_icon_fetch", R"(
        semantics {
          sender: "Startup CRX Install"
          description: "Fetches extension icon images for the startup CRX install UI."
          trigger: "User visits wootzapp://startup-crx-install page."
          data: "No user data is sent."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting: "This request cannot be disabled."
          policy_exception_justification: "Essential for extension installation UI."
        })");

  Profile* profile = Profile::FromWebUI(web_ui_);
  if (!profile) {
    LOG(ERROR) << "Failed to get profile for icon fetch, sending without icon";
    SendExtensionDataToFrontend(name, "", download_url, id, description, version);
    return;
  }

  auto* storage_partition = profile->GetDefaultStoragePartition();
  if (!storage_partition) {
    LOG(ERROR) << "Failed to get storage partition for icon fetch, sending without icon";
    SendExtensionDataToFrontend(name, "", download_url, id, description, version);
    return;
  }

  icon_loader_ = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);
  icon_loader_->DownloadToString(
      storage_partition->GetURLLoaderFactoryForBrowserProcess().get(),
      base::BindOnce(&StartupCrxInstallMessageHandler::OnIconImageFetched,
                     weak_factory_.GetWeakPtr(),
                     name, download_url, id, description, version, icon_url),
      1024 * 1024); // 1MB max size for icon

  LOG(INFO) << "Started icon fetch for: " << name;
}

void StartupCrxInstallMessageHandler::OnIconImageFetched(
    const std::string& name,
    const std::string& download_url,
    const std::string& id,
    const std::string& description,
    const std::string& version,
    const std::string& original_icon_url,
    std::optional<std::string> response_body) {
  
  LOG(INFO) << "OnIconImageFetched called for: " << name;
  
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  std::string icon_base64;
  
  if (response_body && !response_body->empty()) {
    LOG(INFO) << "Icon fetched successfully, size: " << response_body->size();
    
    // Convert response body to a span<const uint8_t> (same as extension store)
    base::span<const uint8_t> input_span(
        reinterpret_cast<const uint8_t*>(response_body->data()),
        response_body->size());

    // Encode the icon as base64 (same method as extension store)
    base::Base64EncodeAppend(input_span, &icon_base64);
    
    LOG(INFO) << "Created base64 icon data, length: " << icon_base64.length();
  } else {
    LOG(WARNING) << "Failed to fetch icon for: " << name << " from URL: " << original_icon_url;
  }
  
  // Send extension data to frontend with base64 icon data (or empty if fetch failed)
  SendExtensionDataToFrontend(name, icon_base64, download_url, id, description, version);
}

void StartupCrxInstallMessageHandler::SendExtensionDataToFrontend(
    const std::string& name,
    const std::string& icon_base64,
    const std::string& download_url,
    const std::string& id,
    const std::string& description,
    const std::string& version) {
  
  if (is_destroyed_) {
    LOG(INFO) << "Handler is destroyed, returning";
    return;
  }
  
  LOG(INFO) << "Sending extension data to frontend: " << name;
  
  // Create a dictionary with all extension data (same pattern as extension store)
  base::Value::Dict extension_data;
  extension_data.Set("name", name);
  extension_data.Set("icon_base64", icon_base64);  // Send base64 data, not data URL
  extension_data.Set("download_url", download_url);
  extension_data.Set("id", id);
  extension_data.Set("description", description);
  extension_data.Set("version", version);
  
  // Send the extension data to JavaScript
  web_ui_->CallJavascriptFunctionUnsafe("handleExtensionData", base::Value(std::move(extension_data)));
  
  LOG(INFO) << "Sent extension data to frontend for: " << name << " with icon base64 length: " 
            << icon_base64.length();
}

// StartupCrxInstallUI implementation
StartupCrxInstallUI::StartupCrxInstallUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui), weak_ptr_factory_(this) {
  LOG(INFO) << "StartupCrxInstallUI constructor called";
  // Set up the wootzapp://startup-crx-install source.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIStartupCrxInstallHost);

  // Add required resources.
  LOG(INFO) << "Adding resources to WebUIDataSource";
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kStartupCrxInstallResources, kStartupCrxInstallResourcesSize),
      IDR_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_HTML);

  // Add the message handler
  web_ui->AddMessageHandler(std::make_unique<StartupCrxInstallMessageHandler>(web_ui));
  LOG(INFO) << "Message handler added to WebUI";

  // As a demonstration of passing a variable for JS to use we pass in some
  // a simple message.
  source->AddString("message", "Hello Startup Crx Install!");
  LOG(INFO) << "Added message string to source";
}

StartupCrxInstallUI::~StartupCrxInstallUI() = default;