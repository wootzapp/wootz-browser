#include "content/browser/copy_paste_blocker/copy_paste_blocked_snackbar_bridge.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/logging.h"
#include "chrome/android/chrome_jni_headers/CopyPasteBlockedSnackbar_jni.h"
#include "content/public/browser/web_contents.h"
#include "ui/android/window_android.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;


CopyPasteBlockedSnackbarBridge::CopyPasteBlockedSnackbarBridge() {
  // No initialization needed
}

CopyPasteBlockedSnackbarBridge::~CopyPasteBlockedSnackbarBridge() {
  // No cleanup needed
}

void CopyPasteBlockedSnackbarBridge::ShowSnackbar(content::WebContents* web_contents,
                                                 const std::string& message) {
  if (!web_contents) {
    LOG(ERROR) << "[CopyPasteBlocker] WebContents is null";
    return;
  }

  // Get the WindowAndroid from WebContents
  ui::WindowAndroid* window_android = web_contents->GetTopLevelNativeWindow();
  if (!window_android) {
    LOG(ERROR) << "[CopyPasteBlocker] WindowAndroid is null";
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  if (!env) {
    LOG(ERROR) << "[CopyPasteBlocker] Failed to attach to JNI environment";
    return;
  }

  // Get the Java WindowAndroid object
  ScopedJavaLocalRef<jobject> java_window_android = window_android->GetJavaObject();
  if (!java_window_android.obj()) {
    LOG(ERROR) << "[CopyPasteBlocker] Failed to get Java WindowAndroid object";
    return;
  }

  // Convert message to Java string
  ScopedJavaLocalRef<jstring> java_message = ConvertUTF8ToJavaString(env, message);
  if (!java_message.obj()) {
    LOG(ERROR) << "[CopyPasteBlocker] Failed to convert message to Java string";
    return;
  }

  // Call the Java method using generated JNI header
  LOG(INFO) << "[CopyPasteBlocker] Calling CopyPasteBlockedSnackbar.show() via generated JNI";
    chrome::Java_CopyPasteBlockedSnackbar_show(
      env, java_window_android, java_message);

  LOG(INFO) << "[CopyPasteBlocker] Successfully called CopyPasteBlockedSnackbar.show() via generated JNI";
} 