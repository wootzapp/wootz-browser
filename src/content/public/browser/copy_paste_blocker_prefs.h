#ifndef CONTENT_PUBLIC_BROWSER_COPY_PASTE_BLOCKER_PREFS_H_
#define CONTENT_PUBLIC_BROWSER_COPY_PASTE_BLOCKER_PREFS_H_

#include "content/common/content_export.h"

class PrefRegistrySimple;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace copy_paste_blocker {
namespace prefs {

CONTENT_EXPORT extern const char kCopyPasteBlockingEnabled[];
CONTENT_EXPORT extern const char kCopyPasteBlockingMode[];
CONTENT_EXPORT extern const char kCopyPasteBlockingDomains[];
CONTENT_EXPORT extern const char kCopyPasteBlockingTypes[];

}

CONTENT_EXPORT void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}

#endif