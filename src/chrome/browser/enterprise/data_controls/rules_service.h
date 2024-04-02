// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ENTERPRISE_DATA_CONTROLS_RULES_SERVICE_H_
#define CHROME_BROWSER_ENTERPRISE_DATA_CONTROLS_RULES_SERVICE_H_

#include "chrome/browser/enterprise/data_controls/chrome_dlp_rules_manager.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"
#include "components/enterprise/data_controls/verdict.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/clipboard_types.h"

namespace base {
template <typename T>
class NoDestructor;
}

namespace data_controls {

// Keyed service that provides an interface to check what restrictions should
// be applied from the DataControlsRules policy.
class RulesService : public KeyedService {
 public:
  ~RulesService() override;

  Verdict GetPrintVerdict(const GURL& printed_page_url) const;
  Verdict GetPasteVerdict(const content::ClipboardEndpoint& source,
                          const content::ClipboardEndpoint& destination,
                          const content::ClipboardMetadata& metadata) const;

  // Returns a clipboard verdict only based the source of the copy, without
  // making any special destination assumptions. This is meant to trigger rules
  // that only have "sources" conditions, and blocking/warning verdicts returned
  // by this function should trigger a dialog.
  Verdict GetCopyRestrictedBySourceVerdict(const GURL& source) const;

  // Returns a clipboard verdict with the provided source attributes, and with
  // the "os_clipboard" destination. This is meant to trigger rules that make
  // use of the "os_clipboard" destination attribute. Blocking verdicts returned
  // by this function should replace the data put in the clipboard, and warning
  // verdicts should trigger a dialog.
  Verdict GetCopyToOSClipboardVerdict(const GURL& source) const;

 protected:
  friend class RulesServiceFactory;

  explicit RulesService(content::BrowserContext* browser_context);

 private:
  // Helpers to convert action-specific types to rule-specific types.
  ActionSource GetAsActionSource(
      const content::ClipboardEndpoint& endpoint) const;
  ActionDestination GetAsActionDestination(
      const content::ClipboardEndpoint& endpoint) const;
  template <typename ActionSourceOrDestination>
  ActionSourceOrDestination ExtractPasteActionContext(
      const content::ClipboardEndpoint& endpoint) const;

  // `profile_` and `rules_manager_` are initialized with the browser_context
  // passed in the constructor.
  const raw_ptr<Profile> profile_ = nullptr;
  ChromeDlpRulesManager rules_manager_;
};

class RulesServiceFactory : public ProfileKeyedServiceFactory {
 public:
  static RulesService* GetForBrowserContext(content::BrowserContext* context);

  static RulesServiceFactory* GetInstance();

  RulesServiceFactory(const RulesServiceFactory&) = delete;
  RulesServiceFactory& operator=(const RulesServiceFactory&) = delete;

 private:
  friend base::NoDestructor<RulesServiceFactory>;

  RulesServiceFactory();
  ~RulesServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace data_controls

#endif  // CHROME_BROWSER_ENTERPRISE_DATA_CONTROLS_RULES_SERVICE_H_
