// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_prefs.h"

#include "components/prefs/pref_registry_simple.h"

namespace wootz_offline_pages {
namespace prefs {

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  // Register offline browsing preference, default to disabled
  registry->RegisterBooleanPref(kOfflineBrowsingEnabled, false);
}

}  // namespace prefs
}  // namespace wootz_offline_pages

