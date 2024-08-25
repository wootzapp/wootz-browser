// Copyright 2021 The Ungoogled Chromium Authors. All rights reserved.
//
// This file is part of Ungoogled Chromium Android.
//
// Ungoogled Chromium Android is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or any later version.
//
// Ungoogled Chromium Android is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Ungoogled Chromium Android.  If not,
// see <https://www.gnu.org/licenses/>.

#ifndef CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_INSTALL_DIALOG_VIEW_ANDROID_H_
#define CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_INSTALL_DIALOG_VIEW_ANDROID_H_

#include <vector>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/timer/elapsed_timer.h"
#include "base/timer/timer.h"
#include "chrome/browser/extensions/extension_install_prompt.h"
#include "ui/gfx/native_widget_types.h"

class Profile;

namespace content {
class PageNavigator;
}

// Modal dialog that shows when the user attempts to install an extension. Also
// shown if the extension is already installed but needs additional permissions.
// Android JNI version.
class ExtensionInstallDialogView {
 public:
  // The views::View::id of the ratings section in the dialog.
  static const int kRatingsViewId = 1;

  ExtensionInstallDialogView(
      Profile* profile,
      content::PageNavigator* navigator,
      const ExtensionInstallPrompt::DoneCallback& done_callback,
     std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt);
+  ~ExtensionInstallDialogView();

  void Cancel(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj);
  void Accept(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    const jboolean withhold_permissions_checkbox_checked);
  void ShowDialog(gfx::NativeWindow native_window);

 private:
  bool is_external_install() const {
    return prompt_->type() == ExtensionInstallPrompt::EXTERNAL_INSTALL_PROMPT;
  }

  // Updates the histogram that holds installation accepted/aborted data.
  void UpdateInstallResultHistogram(bool accepted) const;

  Profile* profile_;
  content::PageNavigator* navigator_;
  ExtensionInstallPrompt::DoneCallback done_callback_;
  std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt_;
  base::string16 title_;

  // Set to true once the user's selection has been received and the callback
 // has been run.
  bool handled_result_;

  // Used to record time between dialog creation and acceptance, cancellation,
  // or dismissal.
  base::Optional<base::ElapsedTimer> install_result_timer_;

  // TODO: Checkbox used to indicate if permissions should be withheld on install.

  // JNI
  bool is_dialog_showing_;
  base::android::ScopedJavaGlobalRef<jobject> java_obj_;

  base::android::ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfPermissions(
      JNIEnv* env);
  base::android::ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfPermissionsDetails(
      JNIEnv* env);
  base::android::ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfRetainedFiles(
      JNIEnv* env);
  base::android::ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfRetainedDeviceMessageStrings(
      JNIEnv* env);

  DISALLOW_COPY_AND_ASSIGN(ExtensionInstallDialogView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_INSTALL_DIALOG_VIEW_H_