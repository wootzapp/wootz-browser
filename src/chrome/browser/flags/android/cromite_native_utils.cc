#include "chrome/browser/flags/android/cromite_native_utils.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/flags/jni_headers/CromiteNativeUtils_jni.h"
#include "components/flags_ui/pref_service_flags_storage.h"

#include "base/android/jni_string.h"

using base::android::JavaParamRef;

static void JNI_CromiteNativeUtils_SetEnabled(JNIEnv* env,
    const JavaParamRef<jstring>& featureName, jboolean isEnabled) {
  flags_ui::PrefServiceFlagsStorage flags_storage(
      g_browser_process->local_state());
  std::set<std::string> entries = flags_storage.GetFlags();

  auto internal_name = base::android::ConvertJavaStringToUTF8(env, featureName);
  entries.erase(internal_name);
  entries.erase(internal_name + "@1"); // enabled
  entries.erase(internal_name + "@2"); // disabled

  if (isEnabled)
    entries.insert(internal_name + "@1");
  else
    entries.insert(internal_name + "@2");

  flags_storage.SetFlags(entries);
  flags_storage.CommitPendingWrites();
}

static jboolean JNI_CromiteNativeUtils_IsFlagEnabled(JNIEnv* env,
    const JavaParamRef<jstring>& featureName) {
  auto internal_name = base::android::ConvertJavaStringToUTF8(env, featureName);
  const std::string enabled_entry = internal_name + "@1";

  flags_ui::PrefServiceFlagsStorage flags_storage(
      g_browser_process->local_state());
  std::set<std::string> entries = flags_storage.GetFlags();
  if (entries.count(enabled_entry))
    return true;

  return false;
}
