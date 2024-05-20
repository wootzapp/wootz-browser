// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_LENS_LENS_PERMISSION_BUBBLE_CONTROLLER_H_
#define CHROME_BROWSER_UI_LENS_LENS_PERMISSION_BUBBLE_CONTROLLER_H_

#include <memory>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "components/prefs/pref_change_registrar.h"

namespace content {
class WebContents;
}

namespace ui {
class DialogModel;
class Event;
}  // namespace ui

namespace views {
class Widget;
}

class BrowserWindowInterface;
class PrefService;

namespace lens {

static constexpr char kLensPermissionDialogName[] = "LensPermissionDialog";

// Manages the Lens Permission Bubble instance for the associated browser.
class LensPermissionBubbleController {
 public:
  // Enumerates the user interactions with the Lens Permission Bubble.
  //
  // These values are persisted to logs. Entries should not be renumbered and
  // numeric values should never be reused.
  //
  // LINT.IfChange(UserAction)
  enum class UserAction {
    // User opened the Help Center link.
    kLinkOpened = 0,
    // User pressed the Accept button.
    kAcceptButtonPressed = 1,
    // User pressed the Cancel button.
    kCancelButtonPressed = 2,
    // User pressed the Esc key.
    kEscKeyPressed = 3,
    kMaxValue = kEscKeyPressed
  };
  // LINT.ThenChange(//tools/metrics/histograms/metadata/others/enums.xml:LensPermissionBubbleUserAction)

  LensPermissionBubbleController(
      BrowserWindowInterface* browser_window_interface,
      PrefService* pref_service);
  LensPermissionBubbleController(const LensPermissionBubbleController&) =
      delete;
  LensPermissionBubbleController& operator=(
      const LensPermissionBubbleController&) = delete;
  ~LensPermissionBubbleController();

  // Shows a tab-modal dialog. `callback` is called when the permission is
  // granted, whether by user directly accepting this dialog or indirectly via
  // pref change.
  using RequestPermissionCallback = base::RepeatingClosure;
  void RequestPermission(content::WebContents* web_contents,
                         RequestPermissionCallback callback);

  // Returns whether there is an associated open dialog widget.
  bool HasOpenDialogWidget();

 private:
  std::unique_ptr<ui::DialogModel> CreateLensPermissionDialogModel();
  void OnHelpCenterLinkClicked(const ui::Event& event);
  void OnPermissionDialogAccept();
  void OnPermissionDialogCancel();
  void OnPermissionDialogClose();
  void OnPermissionPreferenceUpdated(RequestPermissionCallback callback);

  // The associated browser.
  raw_ptr<BrowserWindowInterface> browser_window_interface_ = nullptr;
  // The pref service associated with the current profile.
  raw_ptr<PrefService> pref_service_ = nullptr;
  // Registrar for pref change notifications.
  PrefChangeRegistrar pref_observer_;
  // Pointer to the widget that contains the current open dialog, if any.
  raw_ptr<views::Widget> dialog_widget_ = nullptr;

  base::WeakPtrFactory<LensPermissionBubbleController> weak_ptr_factory_{this};
};

}  // namespace lens

#endif  // CHROME_BROWSER_UI_LENS_LENS_PERMISSION_BUBBLE_CONTROLLER_H_
