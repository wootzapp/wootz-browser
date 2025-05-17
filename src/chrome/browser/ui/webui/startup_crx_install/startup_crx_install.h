#ifndef CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_H_
#define CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_H_

#include <memory>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"

namespace content {
class WebUI;
}

// The message handler for chrome://startup-crx-install
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

  raw_ptr<content::WebUI> web_ui_;
  bool is_destroyed_ = false;
  base::WeakPtrFactory<StartupCrxInstallMessageHandler> weak_factory_;
};

// The WebUI controller for chrome://startup-crx-install
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