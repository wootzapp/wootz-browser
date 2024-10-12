// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_LOGIN_SCREENS_CATEGORIES_SELECTION_SCREEN_H_
#define CHROME_BROWSER_ASH_LOGIN_SCREENS_CATEGORIES_SELECTION_SCREEN_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_discovery_service.h"
#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_discovery_service_factory.h"
#include "chrome/browser/ash/login/screens/base_screen.h"

namespace ash {
class CategoriesSelectionScreenView;

// Controller for the categories selection screen.
class CategoriesSelectionScreen : public BaseScreen {
 public:
  using TView = CategoriesSelectionScreenView;

  enum class Result { kNext, kSkip, kError, kNotApplicable };

  using ScreenExitCallback = base::RepeatingCallback<void(Result result)>;

  CategoriesSelectionScreen(base::WeakPtr<CategoriesSelectionScreenView> view,
                            const ScreenExitCallback& exit_callback);

  CategoriesSelectionScreen(const CategoriesSelectionScreen&) = delete;
  CategoriesSelectionScreen& operator=(const CategoriesSelectionScreen&) =
      delete;

  ~CategoriesSelectionScreen() override;

  static std::string GetResultString(Result result);

 private:
  // BaseScreen:
  bool MaybeSkip(WizardContext& context) override;
  void ShowImpl() override;
  void HideImpl() override;
  void OnUserAction(const base::Value::List& args) override;

  void OnResponseReceived(const std::vector<OOBEAppDefinition>& appInfos,
                          const std::vector<OOBEDeviceUseCase>& categories,
                          AppsFetchingResult result);

  // Called when the user selects categories on the screen.
  void OnSelect(base::Value::List screens);

  base::WeakPtr<CategoriesSelectionScreenView> view_;
  ScreenExitCallback exit_callback_;

  base::WeakPtrFactory<CategoriesSelectionScreen> weak_factory_{this};
};

}  // namespace ash

#endif  // CHROME_BROWSER_ASH_LOGIN_SCREENS_CATEGORIES_SELECTION_SCREEN_H_
