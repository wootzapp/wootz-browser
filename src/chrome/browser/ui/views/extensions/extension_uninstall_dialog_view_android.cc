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

#include <memory>

#include "chrome/browser/ui/views/extensions/extension_uninstall_dialog_view_android.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/android/chrome_jni_headers/ExtensionUninstallDialogBridge_jni.h"
#include "chrome/browser/extensions/extension_uninstall_dialog.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "chrome/browser/ui/ui_features.h"
#include "chrome/grit/generated_resources.h"
#include "components/strings/grit/components_strings.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/android/window_android.h"

ExtensionUninstallDialogViews::ExtensionUninstallDialogViews(
    Profile* profile,
    gfx::NativeWindow parent,
    extensions::ExtensionUninstallDialog::Delegate* delegate)
    : extensions::ExtensionUninstallDialog(profile, parent, delegate) {}

ExtensionUninstallDialogViews::~ExtensionUninstallDialogViews() {
  // Close the widget (the views framework will delete view_).
  if (view_) {
    view_->DialogDestroyed();
  }
}

void ExtensionUninstallDialogViews::Show() {
  view_ = new ExtensionUninstallDialogDelegateView(
      this, extension(), triggering_extension());
  view_->Show(parent());
}

void ExtensionUninstallDialogViews::DialogDelegateDestroyed() {
  // Checks view_ to ensure OnDialogClosed() will not be called twice.
  if (view_) {
    view_ = nullptr;
    OnDialogClosed(CLOSE_ACTION_CANCELED);
  }
}

void ExtensionUninstallDialogViews::DialogAccepted(bool checkbox_checked) {
  // The widget gets destroyed when the dialog is accepted.
  DCHECK(view_);
  view_->DialogDestroyed();
  view_ = nullptr;

  OnDialogClosed(checkbox_checked ? CLOSE_ACTION_UNINSTALL_AND_CHECKBOX_CHECKED
                                  : CLOSE_ACTION_UNINSTALL);
}

void ExtensionUninstallDialogViews::DialogCanceled() {
  // The widget gets destroyed when the dialog is canceled.
  DCHECK(view_);
  view_->DialogDestroyed();
  view_ = nullptr;
  OnDialogClosed(CLOSE_ACTION_CANCELED);
}

ExtensionUninstallDialogDelegateView::ExtensionUninstallDialogDelegateView(
    ExtensionUninstallDialogViews* dialog_view,
    const extensions::Extension* extension,
    const extensions::Extension* triggering_extension)
    : dialog_(dialog_view),
      extension_name_(base::UTF8ToUTF16(extension->name())),
      checkbox_(false) {
  JNIEnv* env = base::android::AttachCurrentThread();
  java_obj_.Reset(env, Java_ExtensionUninstallDialogBridge_create(
    env, reinterpret_cast<intptr_t>(this)).obj());
  DCHECK(!java_obj_.is_null());

  if (triggering_extension) {
    heading_ = l10n_util::GetStringFUTF16(
            IDS_EXTENSION_PROMPT_UNINSTALL_TRIGGERED_BY_EXTENSION,
            base::UTF8ToUTF16(triggering_extension->name()));
  }

  if (dialog_->ShouldShowCheckbox()) {
    checkbox_ = true;
  }

  chrome::RecordDialogCreation(chrome::DialogIdentifier::EXTENSION_UNINSTALL);
}

ExtensionUninstallDialogDelegateView::~ExtensionUninstallDialogDelegateView() {
  // If we're here, 2 things could have happened. Either the user closed the
  // dialog nicely and one of the installed/canceled methods has been called
  // (in which case dialog_ will be null), *or* neither of them have been
  // called and we are being forced closed by our parent widget. In this case,
  // we need to make sure to notify dialog_ not to call us again, since we're
  // about to be freed by the Widget framework.
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ExtensionUninstallDialogBridge_destroy(env, java_obj_);
  if (dialog_)
    dialog_->DialogDelegateDestroyed();
}

const char* ExtensionUninstallDialogDelegateView::GetClassName() const {
  return "ExtensionUninstallDialogDelegateView";
}

void ExtensionUninstallDialogDelegateView::Accept(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      const jboolean jcheckbox_checked) {
  bool checkbox_checked =
    static_cast<bool>(jcheckbox_checked != JNI_FALSE);
  if (dialog_)
    dialog_->DialogAccepted(checkbox_ && checkbox_checked);
}

void ExtensionUninstallDialogDelegateView::Cancel(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj) {
  if (dialog_)
    dialog_->DialogCanceled();
}

void ExtensionUninstallDialogDelegateView::Show(
      gfx::NativeWindow native_window) const {
  if (!native_window) {
    LOG(ERROR) << "extension_uninstall_dialog_view_android.cc: Failed to get native window";
    return;
  }

  // Call java method to create the dialog
  base::string16 checkbox_label = base::UTF8ToUTF16("");
  if (dialog_->ShouldShowCheckbox()) {
    checkbox_label = dialog_->GetCheckboxLabel();
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ExtensionUninstallDialogBridge_show(
    env, java_obj_,
    native_window->GetJavaObject(),
    base::android::ConvertUTF16ToJavaString(env, extension_name_),
    base::android::ConvertUTF16ToJavaString(env, GetWindowTitle()),
    base::android::ConvertUTF16ToJavaString(env, heading_),
    static_cast<bool>(checkbox_),
    base::android::ConvertUTF16ToJavaString(env, checkbox_label)
  );
}

base::string16 ExtensionUninstallDialogDelegateView::GetWindowTitle() const {
  return l10n_util::GetStringFUTF16(IDS_EXTENSION_PROMPT_UNINSTALL_TITLE,
                                    extension_name_);
}

// static
std::unique_ptr<extensions::ExtensionUninstallDialog>
extensions::ExtensionUninstallDialog::Create(Profile* profile,
                                             gfx::NativeWindow parent,
                                             Delegate* delegate) {
  return CreateViews(profile, parent, delegate);
}

// static
std::unique_ptr<extensions::ExtensionUninstallDialog>
extensions::ExtensionUninstallDialog::CreateViews(Profile* profile,
                                                  gfx::NativeWindow parent,
                                                  Delegate* delegate) {
  return std::make_unique<ExtensionUninstallDialogViews>(profile, parent,
                                                         delegate);
}