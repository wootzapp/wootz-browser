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

#include "chrome/browser/ui/views/extensions/extension_install_dialog_view_android.h"

#include <string>
#include <utility>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/bind.h"
#include "base/i18n/message_formatter.h"
#include "base/macros.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/android/chrome_jni_headers/ExtensionInstallDialogViewBridge_jni.h"
#include "chrome/browser/extensions/extension_install_prompt_show_params.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "chrome/browser/ui/scoped_tabbed_browser_displayer.h"
#include "chrome/browser/ui/views/chrome_layout_provider.h"
#include "chrome/browser/ui/views/chrome_typography.h"
#include "chrome/browser/ui/views/extensions/expandable_container_view.h"
#include "chrome/browser/ui/views/extensions/extension_permissions_view.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/grit/generated_resources.h"
#include "components/constrained_window/constrained_window_views.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_urls.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/android/window_android.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/android/gurl_android.h"

using content::OpenURLParams;
using content::Referrer;

namespace {

// Time delay before the install button is enabled after initial display.
int g_install_delay_in_ms = 500;

void ShowExtensionInstallDialogImpl(
    ExtensionInstallPromptShowParams* show_params,
    const ExtensionInstallPrompt::DoneCallback& done_callback,
    std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  ExtensionInstallDialogView* dialog = new ExtensionInstallDialogView(
      show_params->profile(), show_params->GetParentWebContents(),
      done_callback, std::move(prompt));
  dialog->ShowDialog(
    show_params->GetParentWindow());
}

}  // namespace

ExtensionInstallDialogView::ExtensionInstallDialogView(
    Profile* profile,
    content::PageNavigator* navigator,
    const ExtensionInstallPrompt::DoneCallback& done_callback,
    std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt)
    : profile_(profile),
      navigator_(navigator),
      done_callback_(done_callback),
      prompt_(std::move(prompt)),
      title_(prompt_->GetDialogTitle()),
      handled_result_(false),
      is_dialog_showing_(false) {
  DCHECK(prompt_->extension());

  JNIEnv* env = base::android::AttachCurrentThread();
  java_obj_.Reset(env, Java_ExtensionInstallDialogViewBridge_create(
    env, reinterpret_cast<intptr_t>(this)).obj());
  DCHECK(!java_obj_.is_null());

  UMA_HISTOGRAM_ENUMERATION("Extensions.InstallPrompt.Type2", prompt_->type(),
                            ExtensionInstallPrompt::NUM_PROMPT_TYPES);
  chrome::RecordDialogCreation(chrome::DialogIdentifier::EXTENSION_INSTALL);
}

ExtensionInstallDialogView::~ExtensionInstallDialogView() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ExtensionInstallDialogViewBridge_destroy(env, java_obj_);
  if (!handled_result_ && !done_callback_.is_null()) {
    std::move(done_callback_)
        .Run(ExtensionInstallPrompt::Result::USER_CANCELED);
  }
}

// TODO
void ExtensionInstallDialogView::Cancel(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  if (handled_result_)
    return;

  handled_result_ = true;
  UpdateInstallResultHistogram(false);
  std::move(done_callback_).Run(ExtensionInstallPrompt::Result::USER_CANCELED);
}

// TODO
void ExtensionInstallDialogView::Accept(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    jboolean jwithhold_permissions_checkbox_checked) {
  DCHECK(!handled_result_);

  bool withhold_permissions_checkbox_checked =
    static_cast<bool>(jwithhold_permissions_checkbox_checked != JNI_FALSE);
  handled_result_ = true;
  UpdateInstallResultHistogram(true);
  // If the prompt had a checkbox element and it was checked we send that along
  // as the result, otherwise we just send a normal accepted result.
  auto result =
      prompt_->ShouldDisplayWithholdingUI() &&
              withhold_permissions_checkbox_checked
          ? ExtensionInstallPrompt::Result::ACCEPTED_AND_OPTION_CHECKED
          : ExtensionInstallPrompt::Result::ACCEPTED;
  std::move(done_callback_).Run(result);
}

void ExtensionInstallDialogView::ShowDialog(
    gfx::NativeWindow native_window) {
  if (!native_window)
    return;

  base::string16 store_link(base::ASCIIToUTF16(""));
  base::string16 withhold_permissions_heading(base::ASCIIToUTF16(""));
  if (prompt_->has_webstore_data()) {
    // TODO: this is not shown yet
    store_link =
        l10n_util::GetStringUTF16(IDS_EXTENSION_PROMPT_STORE_LINK);
    GURL store_url(extension_urls::GetWebstoreItemDetailURLPrefix() +
                 prompt_->extension()->id());
  } else if (prompt_->ShouldDisplayWithholdingUI()) {
    withhold_permissions_heading =
        l10n_util::GetStringUTF16(IDS_EXTENSION_WITHHOLD_PERMISSIONS);
  }

  // If dialog is already showing, do nothing. (assuming cancel?)
  if (is_dialog_showing_) {
    return;
  }

  is_dialog_showing_ = true;

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_ExtensionInstallDialogViewBridge_showDialog(
      env, java_obj_, native_window->GetJavaObject(),
      base::android::ConvertUTF16ToJavaString(env, title_),
      base::android::ConvertUTF16ToJavaString(env, store_link),
      base::android::ConvertUTF16ToJavaString(env, withhold_permissions_heading),
      static_cast<bool>(prompt_->ShouldDisplayWithholdingUI()),
      static_cast<bool>(prompt_->ShouldShowPermissions()),
      static_cast<int>(prompt_->GetPermissionCount()),
      ToJavaArrayOfPermissions(env),
      ToJavaArrayOfPermissionsDetails(env),
      prompt_->ShouldShowPermissions() ? base::android::ConvertUTF16ToJavaString(env, prompt_->GetPermissionsHeading()) :
      base::android::ConvertUTF16ToJavaString(env, l10n_util::GetStringUTF16(IDS_EXTENSION_NO_SPECIAL_PERMISSIONS)),
      ToJavaArrayOfRetainedFiles(env),
      base::android::ConvertUTF16ToJavaString(env, prompt_->GetRetainedFilesHeading()),
      ToJavaArrayOfRetainedDeviceMessageStrings(env),
      base::android::ConvertUTF16ToJavaString(env, prompt_->GetRetainedDevicesHeading())
      );
}

base::android::ScopedJavaLocalRef<jobjectArray> ExtensionInstallDialogView::ToJavaArrayOfPermissions(
      JNIEnv* env) {
  DCHECK_GE(prompt_->GetPermissionCount(), 0u);
  base::android::ScopedJavaLocalRef<jclass> object_clazz =
    base::android::GetClass(env, "java/lang/String");
  jobjectArray joa =
    env->NewObjectArray(prompt_->GetPermissionCount(), object_clazz.obj(), NULL);
  jni_generator::CheckException(env);

  for (size_t i = 0; i < prompt_->GetPermissionCount(); ++i) {
    env->SetObjectArrayElement(joa, i,
      base::android::ConvertUTF16ToJavaString(env,
        prompt_->GetPermission(i)).obj());
  }
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, joa);
}

base::android::ScopedJavaLocalRef<jobjectArray> ExtensionInstallDialogView::ToJavaArrayOfPermissionsDetails(
  JNIEnv* env) {
  DCHECK_GE(prompt_->GetPermissionCount(), 0u);
  base::android::ScopedJavaLocalRef<jclass> object_clazz =
    base::android::GetClass(env, "java/lang/String");
  jobjectArray joa =
    env->NewObjectArray(prompt_->GetPermissionCount(), object_clazz.obj(), NULL);
  jni_generator::CheckException(env);

  for (size_t i = 0; i < prompt_->GetPermissionCount(); ++i) {
    env->SetObjectArrayElement(joa, i,
      base::android::ConvertUTF16ToJavaString(env,
        prompt_->GetPermissionsDetails(i)).obj());
  }
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, joa);
}

base::android::ScopedJavaLocalRef<jobjectArray> ExtensionInstallDialogView::ToJavaArrayOfRetainedFiles(
  JNIEnv* env) {
  DCHECK_GE(prompt_->GetRetainedFileCount(), 0u);
  base::android::ScopedJavaLocalRef<jclass> object_clazz =
    base::android::GetClass(env, "java/lang/String");
  jobjectArray joa =
    env->NewObjectArray(prompt_->GetRetainedFileCount(), object_clazz.obj(), NULL);
  jni_generator::CheckException(env);

  for (size_t i = 0; i < prompt_->GetRetainedFileCount(); ++i) {
    env->SetObjectArrayElement(joa, i,
      base::android::ConvertUTF16ToJavaString(env,
        prompt_->GetRetainedFile(i)).obj());
  }
  return base::android::ScopedJavaLocalRef<jobjectArray>(env, joa);
}

base::android::ScopedJavaLocalRef<jobjectArray> ExtensionInstallDialogView::ToJavaArrayOfRetainedDeviceMessageStrings(
  JNIEnv* env) {
  DCHECK_GE(prompt_->GetRetainedDeviceCount(), 0u);
+  base::android::ScopedJavaLocalRef<jclass> object_clazz =
+    base::android::GetClass(env, "java/lang/String");
+  jobjectArray joa =
+    env->NewObjectArray(prompt_->GetRetainedDeviceCount(), object_clazz.obj(), NULL);
+  jni_generator::CheckException(env);
+
+  for (size_t i = 0; i < prompt_->GetRetainedDeviceCount(); ++i) {
+    env->SetObjectArrayElement(joa, i,
+      base::android::ConvertUTF16ToJavaString(env,
+        prompt_->GetRetainedDeviceMessageString(i)).obj());
+  }
+  return base::android::ScopedJavaLocalRef<jobjectArray>(env, joa);
+}
+
void ExtensionInstallDialogView::UpdateInstallResultHistogram(bool accepted)
    const {
  // Only update histograms if |install_result_timer_| was initialized in
  // |VisibilityChanged|.
  if (prompt_->type() == ExtensionInstallPrompt::INSTALL_PROMPT &&
      install_result_timer_) {
    if (accepted) {
      UmaHistogramMediumTimes("Extensions.InstallPrompt.TimeToInstall",
                              install_result_timer_->Elapsed());
    } else {
      UmaHistogramMediumTimes("Extensions.InstallPrompt.TimeToCancel",
                              install_result_timer_->Elapsed());
    }
  }
}

// static
ExtensionInstallPrompt::ShowDialogCallback
ExtensionInstallPrompt::GetDefaultShowDialogCallback() {
  return base::Bind(&ShowExtensionInstallDialogImpl);
}