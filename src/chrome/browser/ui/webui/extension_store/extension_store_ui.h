#ifndef CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_

#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"

class ExtensionStoreUI : public content::WebUIController,
                         public content::WebUIMessageHandler {
 public:
  explicit ExtensionStoreUI(content::WebUI* web_ui);
  ~ExtensionStoreUI() override;

  // WebUIMessageHandler overrides.
  void RegisterMessages() override;

 private:
  void HandleFetchExtensions(const base::Value::List& args);
  void OnFetchExtensionsComplete(std::unique_ptr<std::string> response_body);
  void FetchIcon(const std::string& icon_url, base::Value::Dict extension_dict);
  void FetchDownloadUrl(const std::string& download_url, base::Value::Dict extension_dict);
  void OnFetchDownloadUrlComplete(base::Value::Dict extension_dict, std::unique_ptr<std::string> response_body);
  void OnFetchIconComplete(base::Value::Dict extension_dict, std::unique_ptr<std::string> response_body);

  base::WeakPtrFactory<ExtensionStoreUI> weak_factory_{this};
};

#endif  // CHROME_BROWSER_UI_WEBUI_EXTENSION_STORE_EXTENSION_STORE_UI_H_