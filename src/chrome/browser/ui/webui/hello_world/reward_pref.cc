// Copyright 2024 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/hello_world/reward_pref.h"
#include "components/prefs/pref_registry_simple.h"

namespace throttle_webui {

namespace prefs {
    const char kArtifactsJwtToken[] = "rewards.token";
    
}  // namespace prefs

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
    registry->RegisterStringPref(prefs::kArtifactsJwtToken, "");
   
}


 }  