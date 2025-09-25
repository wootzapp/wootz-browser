#ifndef CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_H_
#define CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace content {
class WebUI;
}

namespace network {
class SimpleURLLoader;
}

// The message handler for wootzapp://startup-crx-install
class StartupCrxInstallMessageHandler : public content::WebUIMessageHandler {
 public:
  explicit StartupCrxInstallMessageHandler(content::WebUI* web_ui);
  ~StartupCrxInstallMessageHandler() override;

  StartupCrxInstallMessageHandler(const StartupCrxInstallMessageHandler&) = delete;
  StartupCrxInstallMessageHandler& operator=(const StartupCrxInstallMessageHandler&) = delete;

  // WebUIMessageHandler implementation
  void RegisterMessages() override;
  void OnJavascriptDisallowed() override;

 private:
  // Handles the message for fetching installed extensions.
  void HandleFetchInstalledExtensions(const base::Value::List& args);

  // Handles the message for downloading an extension.
  void HandleDownloadArtifactExtension(const base::Value::List& args);
  
  void SendUtmToFrontend(const std::string& utm_source);

  void HandleGetUtmSource(const base::Value::List& args);
  void HandleGetExtensionData(const base::Value::List& args);

  // Handles the message for installing default extensions.
  void HandleInstallDefaultExtensions(const base::Value::List& args);

  // Checks if this is a first run and should install default extensions
  void CheckForDefaultExtensionInstall();

  // Fetches extensions.json and filters for default_extension = true
  void FetchExtensionsDataForDefaultInstall();

  // Structure to hold default extension information
  struct DefaultExtensionInfo {
    std::string id;
    std::string download_url;
    std::string name;
    std::string description;
    std::string version;
    std::string icon_url;
  };

  // Installs the next default extension in the queue
  void InstallNextDefaultExtension();

  // Callback when an extension installation is complete
  void OnExtensionInstallComplete(const base::Value::List& args);

  // Callback for when default extensions data is fetched
  void OnDefaultExtensionsDataFetched(std::optional<std::string> response_body);

  // Fetches extensions data from GitHub API
  void FetchExtensionsData();
  void OnExtensionsDataFetched(std::optional<std::string> response_body);
  void HandleExtensionsDataFetchError(const std::string& error_message);
  void ProvideFallbackExtensionData(const std::string& utm_source);
  void ParseAndLogExtensionData(const std::string& json_data, const std::string& utm_source);
  
  // Fetches icon image and converts to base64 data URL
  void FetchIconImage(const std::string& name,
                      const std::string& icon_url,
                      const std::string& download_url,
                      const std::string& id,
                      const std::string& description,
                      const std::string& version);
  void OnIconImageFetched(const std::string& name,
                          const std::string& download_url,
                          const std::string& id,
                          const std::string& description,
                          const std::string& version,
                          const std::string& original_icon_url,
                          std::optional<std::string> response_body);
  
  void SendExtensionDataToFrontend(const std::string& name,
                                   const std::string& icon_base64,
                                   const std::string& download_url,
                                   const std::string& id,
                                   const std::string& description,
                                   const std::string& version);

  raw_ptr<content::WebUI> web_ui_;
  bool is_destroyed_ = false;
  std::unique_ptr<network::SimpleURLLoader> extensions_loader_;
  std::unique_ptr<network::SimpleURLLoader> icon_loader_;
  
  // Queue of default extensions to install
  std::vector<DefaultExtensionInfo> default_extensions_queue_;
  
  // Current extension index being installed
  size_t current_extension_index_ = 0;
  
  base::WeakPtrFactory<StartupCrxInstallMessageHandler> weak_factory_;
};

// The WebUI controller for wootzapp://startup-crx-install
class StartupCrxInstallUI : public content::WebUIController {
 public:
  explicit StartupCrxInstallUI(content::WebUI* web_ui);
  ~StartupCrxInstallUI() override;

  StartupCrxInstallUI(const StartupCrxInstallUI&) = delete;
  StartupCrxInstallUI& operator=(const StartupCrxInstallUI&) = delete;

 private:
  base::WeakPtrFactory<StartupCrxInstallUI> weak_ptr_factory_;
};

#endif  // CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_H_