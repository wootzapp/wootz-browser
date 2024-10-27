#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "chrome/browser/extensions/extension_service.h"
#include "extensions/browser/extension_system.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/file_util.h"
#include "base/files/file_util.h"
#include "base/process/internal_linux.h"
#include "extensions/common/api/extension_action/action_info.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/android/chrome_jni_headers/Extensions_jni.h"
#include "base/threading/thread_restrictions.h"

using base::android::ScopedJavaLocalRef;

namespace extensions {
class IconLoaderJNI {
public:
    static jobject GetBitmapFromIconData(JNIEnv* env, const extensions::Extension* extension) {
        base::ScopedAllowBlocking allow_blocking;
        base::FilePath icon_path = extension->GetResource("icons/icon128.png").GetFilePath();
        if (icon_path.empty()) {
            return nullptr;
        }

        std::string icon_bytes;
        if (!base::ReadFileToString(icon_path, &icon_bytes)) {
            return nullptr;
        }

        ScopedJavaLocalRef<jbyteArray> icon_byte_array = base::android::ToJavaByteArray(
            env, reinterpret_cast<const uint8_t*>(icon_bytes.data()), icon_bytes.size());

        jclass bitmap_factory_class = env->FindClass("android/graphics/BitmapFactory");
        jmethodID decode_method = env->GetStaticMethodID(bitmap_factory_class, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;");
        jobject bitmap = env->CallStaticObjectMethod(bitmap_factory_class, decode_method, icon_byte_array.obj(), 0, icon_bytes.size());

        return bitmap;
    }
};
}

static ScopedJavaLocalRef<jobject> JNI_Extensions_GetExtensionsInfo(
    JNIEnv* env) {
    Profile* profile = ProfileManager::GetActiveUserProfile();
    if (!profile) {
        return nullptr;
    }

    extensions::ExtensionRegistry* registry = extensions::ExtensionRegistry::Get(profile);
    if (!registry) {
        return nullptr;
    }

    const extensions::ExtensionSet& installed_extensions = registry->enabled_extensions();

    jclass array_list_class = env->FindClass("java/util/ArrayList");
    jmethodID array_list_init = env->GetMethodID(array_list_class, "<init>", "()V");
    jmethodID array_list_add = env->GetMethodID(array_list_class, "add", "(Ljava/lang/Object;)Z");
    jobject extension_info_list = env->NewObject(array_list_class, array_list_init);

    jclass extension_info_class = env->FindClass("org/chromium/chrome/browser/extensions/ExtensionInfo");
    jmethodID extension_info_constructor = env->GetMethodID(extension_info_class, "<init>",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/graphics/Bitmap;)V");

    for (const auto& extension : installed_extensions) {
        std::string name = extension->name();
        std::string description = extension->description();
        
        const extensions::ActionInfo* action_info = extensions::ActionInfo::GetExtensionActionInfo(extension.get());
        std::string popup_url = (action_info && !action_info->default_popup_url.is_empty())
            ? action_info->default_popup_url.spec() 
            : "";

        jstring jid = base::android::ConvertUTF8ToJavaString(env, extension->id()).Release();
        jstring jname = base::android::ConvertUTF8ToJavaString(env, name).Release();
        jstring jdescription = base::android::ConvertUTF8ToJavaString(env, description).Release();
        jstring jpopup_url = base::android::ConvertUTF8ToJavaString(env, popup_url).Release();

        jobject icon_bitmap = extensions::IconLoaderJNI::GetBitmapFromIconData(env, extension.get());
        // jobject icon_bitmap = nullptr;

        jobject extension_info = env->NewObject(extension_info_class, extension_info_constructor,
            jid, jname, jdescription, jpopup_url, icon_bitmap);
        env->CallBooleanMethod(extension_info_list, array_list_add, extension_info);
    }

    return base::android::ScopedJavaLocalRef<jobject>(env, extension_info_list);
}

void JNI_Extensions_UninstallExtension(JNIEnv* env, const base::android::JavaParamRef<jstring>& j_extension_id) {
    std::string extension_id = base::android::ConvertJavaStringToUTF8(env, j_extension_id);

    Profile* profile = ProfileManager::GetActiveUserProfile();
    if (!profile) {
        return;
    }

    extensions::ExtensionService* extension_service = extensions::ExtensionSystem::Get(profile)->extension_service();
    if (!extension_service) {
        return;
    }

    extension_service->UninstallExtension(extension_id, extensions::UNINSTALL_REASON_USER_INITIATED, nullptr);
}