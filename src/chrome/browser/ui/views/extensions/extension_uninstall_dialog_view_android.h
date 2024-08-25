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

#ifndef CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_VIEW_ANDROID_H_
#define CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_VIEW_ANDROID_H_

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "chrome/browser/extensions/extension_uninstall_dialog.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "ui/gfx/native_widget_types.h"

namespace {
class ExtensionUninstallDialogDelegateView;

// Views implementation of the uninstall dialog.
class ExtensionUninstallDialogViews
    : public extensions::ExtensionUninstallDialog {
 public:
  ExtensionUninstallDialogViews(
      Profile* profile,
      gfx::NativeWindow parent,
      extensions::ExtensionUninstallDialog::Delegate* delegate);
  ~ExtensionUninstallDialogViews() override;

  // Called when the ExtensionUninstallDialogDelegate has been destroyed to make
  // sure we invalidate pointers. This object will also be freed.
  void DialogDelegateDestroyed();

  // Forwards the accept and cancels to the delegate.
  void DialogAccepted(bool checkbox_checked);
  void DialogCanceled();

 private:
  void Show() override;

  ExtensionUninstallDialogDelegateView* view_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ExtensionUninstallDialogViews);
};

// The dialog's view, owned by the views framework.
class ExtensionUninstallDialogDelegateView {
 public:
  // Constructor for view component of dialog. triggering_extension may be null
  // if the uninstall dialog was manually triggered (from chrome://extensions).
  ExtensionUninstallDialogDelegateView(
      ExtensionUninstallDialogViews* dialog_view,
      const extensions::Extension* extension,
      const extensions::Extension* triggering_extension);
  ~ExtensionUninstallDialogDelegateView();

  void Accept(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      const jboolean jcheckbox_checked);
  void Cancel(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj);
  void Show(gfx::NativeWindow native_window) const;
  // Called when the ExtensionUninstallDialog has been destroyed to make sure
  // we invalidate pointers.
  void DialogDestroyed() { dialog_ = NULL; }

 private:
  // views::View:
  const char* GetClassName() const;

  // views::DialogDelegateView:
  base::string16 GetWindowTitle() const;

  ExtensionUninstallDialogViews* dialog_;
  const base::string16 extension_name_;
  bool checkbox_;
  base::string16 heading_;

  base::android::ScopedJavaGlobalRef<jobject> java_obj_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionUninstallDialogDelegateView);
};
}

#endif  // CHROME_BROWSER_UI_VIEWS_EXTENSIONS_EXTENSION_UNINSTALL_DIALOG_VIEW_ANDROID_H_