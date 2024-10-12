// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_PERSONALIZED_RECOMMEND_APPS_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_PERSONALIZED_RECOMMEND_APPS_SCREEN_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_discovery_service.h"
#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_types.h"
#include "chrome/browser/ash/login/screens/base_screen.h"

namespace ash {
class PersonalizedRecommendAppsScreenView;

// Controller for the new recommended apps screen.
class PersonalizedRecommendAppsScreen : public BaseScreen {
 public:
  using TView = PersonalizedRecommendAppsScreenView;

  enum class Result { kNext, kSkip, kBack, kNotApplicable };

  using ScreenExitCallback = base::RepeatingCallback<void(Result result)>;

  PersonalizedRecommendAppsScreen(
      base::WeakPtr<PersonalizedRecommendAppsScreenView> view,
      const ScreenExitCallback& exit_callback);

  PersonalizedRecommendAppsScreen(const PersonalizedRecommendAppsScreen&) =
      delete;
  PersonalizedRecommendAppsScreen& operator=(
      const PersonalizedRecommendAppsScreen&) = delete;

  ~PersonalizedRecommendAppsScreen() override;

  static std::string GetResultString(Result result);

 private:
  // BaseScreen:
  bool MaybeSkip(WizardContext& context) override;
  void ShowImpl() override;
  void HideImpl() override;
  void OnUserAction(const base::Value::List& args) override;

  void OnResponseReceived(const std::vector<OOBEAppDefinition>& app_infos,
                          const std::vector<OOBEDeviceUseCase>& use_cases,
                          AppsFetchingResult result);

  std::vector<OOBEAppDefinition> app_infos_;

  base::WeakPtr<PersonalizedRecommendAppsScreenView> view_;
  ScreenExitCallback exit_callback_;

  base::WeakPtrFactory<PersonalizedRecommendAppsScreen> weak_factory_{this};
};

}  // namespace ash

#endif  // CHROME_BROWSER_ASH_LOGIN_SCREENS_PERSONALIZED_RECOMMEND_APPS_SCREEN_H_
