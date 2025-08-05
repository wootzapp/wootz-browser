#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/base64.h"
#include "base/files/file_util.h"
#include "base/json/json_writer.h"
#include "base/process/internal_linux.h"
#include "base/threading/thread_restrictions.h"
#include "chrome/android/chrome_jni_headers/Extensions_jni.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_icon_image.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/api/extension_action/action_info.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_icon_set.h"
#include "extensions/common/extension_resource.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/file_util.h"
#include "extensions/common/manifest_handlers/icons_handler.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/image/image.h"
#include "components/subresource_filter/core/browser/subresource_filter_prefs.h"
#include "components/prefs/pref_service.h"
#include "base/logging.h"
using base::android::ScopedJavaLocalRef;

namespace extensions {
class IconLoaderJNI {
 public:
  static std::string GetIconBytesBase64(
      const extensions::Extension* extension) {
    const int kPreferredSize = 128;
    const ExtensionIconSet& icons = IconsInfo::GetIcons(extension);
    if (icons.empty()) {
      return "";
    }

    ExtensionIconSet::Match match_type = ExtensionIconSet::Match::kExactly;
    std::string icon_path = icons.Get(kPreferredSize, match_type);
    if (icon_path.empty()) {
      match_type = ExtensionIconSet::Match::kSmaller;
      icon_path = icons.Get(kPreferredSize, match_type);
    }
    if (icon_path.empty()) {
      match_type = ExtensionIconSet::Match::kBigger;
      icon_path = icons.Get(kPreferredSize, match_type);
    }
    if (icon_path.empty()) {
      return "";
    }

    base::ScopedAllowBlocking allow_blocking;
    ExtensionResource resource =
        IconsInfo::GetIconResource(extension, kPreferredSize, match_type);

    base::FilePath file_path = resource.GetFilePath();
    if (file_path.empty()) {
      return "";
    }

    std::string icon_bytes;
    if (!base::ReadFileToString(file_path, &icon_bytes)) {
      return "";
    }

    return base::Base64Encode(base::span<const uint8_t>(
        reinterpret_cast<const uint8_t*>(icon_bytes.data()),
        icon_bytes.size()));
  }
};
}  // namespace extensions

static ScopedJavaLocalRef<jstring> JNI_Extensions_GetExtensionsInfo(
    JNIEnv* env) {
  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    return base::android::ConvertUTF8ToJavaString(env, "[]");
  }

  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(profile);
  if (!registry) {
    return base::android::ConvertUTF8ToJavaString(env, "[]");
  }

  const extensions::ExtensionSet& installed_extensions =
      registry->enabled_extensions();

  base::Value::List extensions_list;

  for (const auto& extension : installed_extensions) {
    base::Value::Dict extension_info;

    extension_info.Set("id", extension->id());
    extension_info.Set("name", extension->name());
    extension_info.Set("description", extension->description());
    extension_info.Set("version", extension->version().GetString());

    const extensions::ActionInfo* action_info =
        extensions::ActionInfo::GetExtensionActionInfo(extension.get());
    std::string popup_url =
        (action_info && !action_info->default_popup_url.is_empty())
            ? action_info->default_popup_url.spec()
            : "";
    extension_info.Set("popup_url", popup_url);

    std::string widget_url =
        (action_info && !action_info->default_widget_url.is_empty())
            ? action_info->default_widget_url.spec()
            : "";
    extension_info.Set("widget_url", widget_url);

    base::Value::List features_list;
    if (action_info) {
      if (!action_info->features.empty()) {
        for (const std::string& feature : action_info->features) {
          features_list.Append(feature);
        }
      } else {
        LOG(ERROR) << "No features found in action_info";
      }
    } else {
      LOG(ERROR) << "No action_info found for extension";
    }
    extension_info.Set("features", base::Value(std::move(features_list)));

    std::string icon_base64 =
        extensions::IconLoaderJNI::GetIconBytesBase64(extension.get());
    extension_info.Set("icon_base64", icon_base64);

    extensions_list.Append(base::Value(std::move(extension_info)));
  }

  std::string json_string;
  base::JSONWriter::Write(base::Value(std::move(extensions_list)),
                          &json_string);
  return base::android::ConvertUTF8ToJavaString(env, json_string);
}

void JNI_Extensions_InstallExtension(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& j_url) {
  std::string url = base::android::ConvertJavaStringToUTF8(env, j_url);
  std::string base_url = url;
  base_url = base::TrimWhitespaceASCII(base_url, base::TRIM_ALL);
  base_url = base::ToLowerASCII(base_url);
  LOG(INFO) << "base_url: " << base_url;
  LOG(INFO) << "url: " << url;
  const std::string kofficialStore = "wootzapp://flow-store";
  const std::string kinitialSource = "wootzapp://startup-crx-install";
  if (base_url != kofficialStore && base_url != kinitialSource) {
    LOG(ERROR) << "Install from official store , WRONG URL = " << url;
    return;
  }
  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    LOG(ERROR) << "No Active Profile FOUND";
    return;
  }

  extensions::ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(profile)->extension_service();

  if (!extension_service) {
    LOG(ERROR) << "Extension Service Not Available";
    return;
  }
}

void JNI_Extensions_UninstallExtension(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& j_extension_id) {
  std::string extension_id =
      base::android::ConvertJavaStringToUTF8(env, j_extension_id);

  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    return;
  }

  const char* kArtifactExtensionName = "Artifact Extension";
  extensions::ExtensionRegistry* registry = extensions::ExtensionRegistry::Get(profile);
  bool is_artifact_extension = false;
  if (registry) {
    for (const auto& extension : registry->enabled_extensions()) {
      if (extension->id() == extension_id && extension->name() == kArtifactExtensionName) {
        is_artifact_extension = true;
        break;
      }
    }
  }

  if (is_artifact_extension) {
    LOG(INFO) << "Disabling AdBlock and Ad Replacement on uninstalling the Artifact Extension";
    PrefService* prefs = profile->GetPrefs();
    prefs->SetBoolean(subresource_filter::prefs::kAdBlockGlobalEnabled, false);
    prefs->SetList(subresource_filter::prefs::kAdReplacementSelectors, base::Value::List());
    prefs->CommitPendingWrite();
  }

  extensions::ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!extension_service) {
    return;
  }

  extension_service->UninstallExtension(
      extension_id, extensions::UNINSTALL_REASON_USER_INITIATED, nullptr);
}
