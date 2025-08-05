#ifndef CHROME_BROWSER_RENDERER_CONTEXT_MENU_COPY_PASTE_BLOCKED_SNACKBAR_BRIDGE_H_
#define CHROME_BROWSER_RENDERER_CONTEXT_MENU_COPY_PASTE_BLOCKED_SNACKBAR_BRIDGE_H_

#include <string>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "ui/android/window_android.h"

namespace content {
class WebContents;
}

/**
 * C++ bridge to the Java CopyPasteBlockedSnackbar.
 * Provides a native Android Snackbar for copy-paste blocking.
 */
class CopyPasteBlockedSnackbarBridge {
 public:
  CopyPasteBlockedSnackbarBridge();
  ~CopyPasteBlockedSnackbarBridge();

  /**
   * Shows a native Android Snackbar with the copy-paste blocked message.
   * 
   * @param web_contents The WebContents instance
   * @param message The message to display
   */
  static void ShowSnackbar(content::WebContents* web_contents, 
                          const std::string& message);

 private:
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
};

#endif  // CHROME_BROWSER_RENDERER_CONTEXT_MENU_COPY_PASTE_BLOCKED_SNACKBAR_BRIDGE_H_ 