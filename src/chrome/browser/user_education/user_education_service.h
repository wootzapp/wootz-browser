// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_USER_EDUCATION_USER_EDUCATION_SERVICE_H_
#define CHROME_BROWSER_USER_EDUCATION_USER_EDUCATION_SERVICE_H_

#include <memory>

#include "chrome/browser/user_education/browser_tutorial_service.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/user_education/common/feature_promo_registry.h"
#include "components/user_education/common/feature_promo_session_manager.h"
#include "components/user_education/common/feature_promo_session_policy.h"
#include "components/user_education/common/feature_promo_storage_service.h"
#include "components/user_education/common/help_bubble_factory_registry.h"
#include "components/user_education/common/new_badge_controller.h"
#include "components/user_education/common/product_messaging_controller.h"
#include "components/user_education/common/tutorial.h"
#include "components/user_education/common/tutorial_registry.h"

extern const char kTabGroupTutorialId[];
extern const char kSavedTabGroupTutorialId[];
extern const char kSidePanelCustomizeChromeTutorialId[];
extern const char kSideSearchTutorialId[];
extern const char kPasswordManagerTutorialId[];

class UserEducationService : public KeyedService {
 public:
  explicit UserEducationService(
      std::unique_ptr<user_education::FeaturePromoStorageService>
          storage_service,
      bool allows_promos);
  ~UserEducationService() override;

  user_education::TutorialRegistry& tutorial_registry() {
    return tutorial_registry_;
  }
  user_education::TutorialService& tutorial_service() {
    return tutorial_service_;
  }
  user_education::HelpBubbleFactoryRegistry& help_bubble_factory_registry() {
    return help_bubble_factory_registry_;
  }
  user_education::FeaturePromoRegistry& feature_promo_registry() {
    return feature_promo_registry_;
  }
  user_education::ProductMessagingController& product_messaging_controller() {
    return product_messaging_controller_;
  }
  user_education::FeaturePromoStorageService& feature_promo_storage_service() {
    return *feature_promo_storage_service_;
  }
  user_education::FeaturePromoSessionManager& feature_promo_session_manager() {
    return feature_promo_session_manager_;
  }
  user_education::FeaturePromoSessionPolicy& feature_promo_session_policy() {
    return *feature_promo_session_policy_;
  }
  user_education::NewBadgeRegistry* new_badge_registry() {
    return new_badge_registry_.get();
  }
  user_education::NewBadgeController* new_badge_controller() {
    return new_badge_controller_.get();
  }

 private:
  user_education::TutorialRegistry tutorial_registry_;
  user_education::HelpBubbleFactoryRegistry help_bubble_factory_registry_;
  user_education::FeaturePromoRegistry feature_promo_registry_;
  BrowserTutorialService tutorial_service_;
  user_education::ProductMessagingController product_messaging_controller_;
  std::unique_ptr<user_education::FeaturePromoStorageService>
      feature_promo_storage_service_;
  user_education::FeaturePromoSessionManager feature_promo_session_manager_;
  std::unique_ptr<user_education::FeaturePromoSessionPolicy>
      feature_promo_session_policy_;
  std::unique_ptr<user_education::NewBadgeRegistry> new_badge_registry_;
  std::unique_ptr<user_education::NewBadgeController> new_badge_controller_;
};

#endif  // CHROME_BROWSER_USER_EDUCATION_USER_EDUCATION_SERVICE_H_
