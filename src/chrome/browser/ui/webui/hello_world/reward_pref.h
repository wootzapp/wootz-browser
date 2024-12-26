// Copyright 2024 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HELLO_WORLD_REWARD_PREF_H_
#define CHROME_BROWSER_UI_WEBUI_HELLO_WORLD_REWARD_PREF_H_

#include "components/prefs/pref_registry_simple.h"

class PrefRegistrySimple;

namespace throttle_webui {
  namespace prefs {
     extern const char kArtifactsJwtToken[];
    
  }  // namespace prefs

  // Registers user preferences related to bookmarks.
  void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // void LoadNetworkThrottlingSettings();
}

#endif  // CHROME_BROWSER_UI_WEBUI_THROTTLE_THROTTLE_PREFS_H_