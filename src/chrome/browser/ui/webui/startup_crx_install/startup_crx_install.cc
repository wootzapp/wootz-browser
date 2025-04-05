#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install.h"

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

// Add this to access shared preferences
#include "components/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"

// Add this include at the top
#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install_prefs.h"

// StartupCrxInstallMessageHandler implementation
StartupCrxInstallMessageHandler::StartupCrxInstallMessageHandler(content::WebUI* web_ui)
    : web_ui_(web_ui),
      weak_factory_(this) {
  LOG(INFO) << "StartupCrxInstallMessageHandler constructor called";
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
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleDownloadArtifactExtension,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered downloadArtifactExtension handler";

  web_ui_->RegisterMessageCallback(
      "getUtmSource",
      base::BindRepeating(&StartupCrxInstallMessageHandler::HandleGetUtmSource,
                         weak_factory_.GetWeakPtr()));
  LOG(INFO) << "Registered getUtmSource handler";
  
  // Add a test message that we can trigger manually
  web_ui_->RegisterMessageCallback(
      "testMessageHandler",
      base::BindRepeating([](const base::Value::List& args) {
        LOG(INFO) << "Test message handler called with " << args.size() << " arguments";
      }));
  LOG(INFO) << "Registered testMessageHandler";
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
    return;
  }
  
  LOG(INFO) << "About to call JavaScript function with UTM source: " << utm_source;
  
  // Try with a simple string value
  base::Value utm_value(utm_source);
  
  // Debug the value being sent
  std::string debug_json;
  base::JSONWriter::Write(utm_value, &debug_json);
  LOG(INFO) << "UTM value as JSON: " << debug_json;
  
  // Call the JavaScript function
  web_ui_->CallJavascriptFunctionUnsafe("handleUtmSource", utm_value);
  
  // Also try with a direct string for testing
  if (utm_source.empty()) {
    LOG(INFO) << "UTM source is empty, sending test value";
    web_ui_->CallJavascriptFunctionUnsafe("handleUtmSource", base::Value("test_utm_value"));
  }
  
  LOG(INFO) << "Sent UTM source to JS: " << utm_source;
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

void StartupCrxInstallMessageHandler::HandleDownloadArtifactExtension(const base::Value::List& args) {
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
  
  // Send progress updates to the frontend
  base::Value::Dict progress;
  progress.Set("state", "downloading");
  progress.Set("percentComplete", 0);
  LOG(INFO) << "Sending progress updates to frontend";
  web_ui_->CallJavascriptFunctionUnsafe(
      "handleDownloadProgress", base::Value(std::move(progress)));
  LOG(INFO) << "Sent progress updates to frontend";
  
  // When download completes, update state
  // base::Value::Dict complete;
  // complete.Set("state", "complete");
  // web_ui_->CallJavascriptFunctionUnsafe(
  //     "handleDownloadProgress", base::Value(std::move(complete)));
}

// StartupCrxInstallUI implementation
StartupCrxInstallUI::StartupCrxInstallUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui), weak_ptr_factory_(this) {
  LOG(INFO) << "StartupCrxInstallUI constructor called";
  // Set up the chrome://startup-crx-install source.
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