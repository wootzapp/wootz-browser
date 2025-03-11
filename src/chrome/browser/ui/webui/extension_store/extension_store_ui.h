#ifndef CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_

#include <map>
#include <memory>
#include <string>

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "components/history/core/browser/history_service_observer.h"
// Commented out observer
// #include "content/public/browser/web_contents_observer.h"

namespace network {
class SimpleURLLoader;
}  // namespace network

// Forward declarations
class ExtensionStoreMessageHandler;

// The WebUI controller for chrome://extension-store
class ExtensionStoreUI : public content::WebUIController {
 public:
  explicit ExtensionStoreUI(content::WebUI* web_ui);
  ~ExtensionStoreUI() override;

  ExtensionStoreUI(const ExtensionStoreUI&) = delete;
  ExtensionStoreUI& operator=(const ExtensionStoreUI&) = delete;
};

// The message handler for chrome://extension-store
class ExtensionStoreMessageHandler : public content::WebUIMessageHandler,
                                      public history::HistoryServiceObserver {
 public:
  explicit ExtensionStoreMessageHandler(content::WebUI* web_ui);
  ~ExtensionStoreMessageHandler() override;

  ExtensionStoreMessageHandler(const ExtensionStoreMessageHandler&) = delete;
  ExtensionStoreMessageHandler& operator=(const ExtensionStoreMessageHandler&) = delete;

  // WebUIMessageHandler implementation
  void RegisterMessages() override;
  void OnJavascriptDisallowed() override;

 private:
  // Handles the message for fetching extensions.
  void HandleFetchExtensions(const base::Value::List& args);

  // Handles the message for fetching an icon.
  void HandleFetchIcon(const base::Value::List& args);
  
  // Handles the message for fetching installed extensions.
  void HandleFetchInstalledExtensions(const base::Value::List& args);

  // Called when the extensions list has been fetched.
  void OnFetchExtensionsComplete(std::unique_ptr<std::string> response_body);

  // Sends a specific extension to the frontend.
  void SendExtensionToFrontend(const std::string& extension_id);

  // Fetches an icon for an extension.
  void FetchIcon(const std::string& icon_url, const std::string& extension_id);

  // Called when an icon has been fetched.
  void OnFetchIconComplete(
      const std::string& extension_id,
      const std::string& loader_key,
      std::unique_ptr<std::string> response_body);

  bool is_destroyed_;
  void CleanupResources();
  // Storage for the extensions data, keyed by extension ID.
  std::map<std::string, base::Value::Dict> extensions_data_;

  // The loader for fetching extensions.
  std::unique_ptr<network::SimpleURLLoader> extensions_loader_;

  // Replace the single icon loader with a map to track multiple concurrent downloads
  std::map<std::string, std::unique_ptr<network::SimpleURLLoader>> icon_loaders_;


  // Store a raw pointer to the WebUI for callbacks
  raw_ptr<content::WebUI> web_ui_;

  // Factory for callbacks.
  base::WeakPtrFactory<ExtensionStoreMessageHandler> weak_factory_;
};

#endif  // CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_