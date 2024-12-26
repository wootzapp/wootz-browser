#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "components/history/core/browser/history_service_observer.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include <memory>

namespace user_prefs {
class PrefRegistrySyncable;
}

class RewardsUI : public content::WebUIController {
 public:
  explicit RewardsUI(content::WebUI* web_ui);

  RewardsUI(const RewardsUI&) = delete;
  RewardsUI& operator=(const RewardsUI&) = delete;
};

class RewardsMessageHandler : public content::WebUIMessageHandler,
                              public history::HistoryServiceObserver {
 public:
    explicit RewardsMessageHandler(content::WebUI* web_ui);

    RewardsMessageHandler(const RewardsMessageHandler&) = delete;
    RewardsMessageHandler& operator=(const RewardsMessageHandler&) = delete;

    ~RewardsMessageHandler() override;

    // WebUIMessageHandler implementation:
    void RegisterMessages() override;
    void OnJavascriptDisallowed() override;

    // HistoryServiceObserver implementation:
    void OnURLVisited(history::HistoryService* history_service,
                      const history::URLRow& url_row,
                      const history::VisitRow& visit_row) override;
    static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);
 private:
    void LoginWallet(const base::Value::List& args);
    void LogUrl(const base::Value::List& args);
    void Encrypt(const base::Value::List& args);
    void GetUserProfile(const base::Value::List& args);
    void OnURLLoadComplete(std::unique_ptr<std::string> response_body);
    void OnGetUserProfileComplete(std::unique_ptr<std::string> response_body);
    void SendStoredApiResponse();

    raw_ptr<content::WebUI> web_ui_;
    std::unique_ptr<network::SimpleURLLoader> simple_loader;
    base::WeakPtrFactory<RewardsMessageHandler> weak_factory_{this};
};