#include "content/public/browser/copy_paste_blocker_prefs.h"

#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_registry_simple.h"

namespace copy_paste_blocker {
namespace prefs {

const char kCopyPasteBlockingEnabled[] = "wootz.copy_paste_blocking.enabled";
const char kCopyPasteBlockingMode[] = "wootz.copy_paste_blocking.mode";
const char kCopyPasteBlockingDomains[] = "wootz.copy_paste_blocking.domains";
const char kCopyPasteBlockingTypes[] = "wootz.copy_paste_blocking.block_types";

}

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kCopyPasteBlockingEnabled, false);
  registry->RegisterStringPref(prefs::kCopyPasteBlockingMode, "blacklist");
  registry->RegisterListPref(prefs::kCopyPasteBlockingDomains);

  // Register block types as a dictionary with default values
  base::Value::Dict default_block_types;
  default_block_types.Set("copy", true);
  default_block_types.Set("cut", true);
  default_block_types.Set("paste", true);
  default_block_types.Set("contextMenu", true);
  default_block_types.Set("selectAll", false);
  default_block_types.Set("dragDrop", true);

  registry->RegisterDictionaryPref(prefs::kCopyPasteBlockingTypes, 
                                  std::move(default_block_types));
}

}