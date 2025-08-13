#include "chrome/browser/renderer_context_menu/copy_paste_blocked_snackbar_bridge.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/logging.h"
#include "content/public/browser/web_contents.h"
#include "ui/android/window_android.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;

namespace {

// JNI method signatures
const char kCopyPasteBlockedSnackbarClass[] =
    "org/chromium/chrome/browser/renderer_context_menu/CopyPasteBlockedSnackbar";

// JNI method to show the Snackbar
const char kShowSnackbarMethod[] = "show";
const char kShowSnackbarSignature[] = "(Lorg/chromium/ui/base/WindowAndroid;Ljava/lang/String;)V";

}  // namespace

CopyPasteBlockedSnackbarBridge::CopyPasteBlockedSnackbarBridge() {
  // No initialization needed
}

CopyPasteBlockedSnackbarBridge::~CopyPasteBlockedSnackbarBridge() {
  // No cleanup needed
}

void CopyPasteBlockedSnackbarBridge::ShowSnackbar(content::WebContents* web_contents,
                                                 const std::string& message) {
  if (!web_contents) {
    LOG(ERROR) << "WebContents is null";
    return;
  }

  // Get the WindowAndroid from WebContents
  ui::WindowAndroid* window_android = web_contents->GetTopLevelNativeWindow();
  if (!window_android) {
    LOG(ERROR) << "WindowAndroid is null";
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  if (!env) {
    LOG(ERROR) << "Failed to attach to JNI environment";
    return;
  }

  // Get the Java class using FindClass
  ScopedJavaLocalRef<jclass> clazz(env, env->FindClass(kCopyPasteBlockedSnackbarClass));
  if (!clazz.obj()) {
    LOG(ERROR) << "Failed to find CopyPasteBlockedSnackbar class";
    return;
  }

  // Get the static method ID
  jmethodID method_id = env->GetStaticMethodID(clazz.obj(), kShowSnackbarMethod, 
                                              kShowSnackbarSignature);
  if (!method_id) {
    LOG(ERROR) << "Failed to find show method";
    return;
  }

  // Convert parameters to Java types
  ScopedJavaLocalRef<jobject> java_window_android = window_android->GetJavaObject();
  ScopedJavaLocalRef<jstring> java_message = ConvertUTF8ToJavaString(env, message);

  // Call the Java method
  env->CallStaticVoidMethod(clazz.obj(), method_id, java_window_android.obj(), 
                           java_message.obj());

  // Check for exceptions
  if (env->ExceptionCheck()) {
    LOG(ERROR) << "Exception occurred while calling Java method";
    env->ExceptionDescribe();
    env->ExceptionClear();
  }
} 