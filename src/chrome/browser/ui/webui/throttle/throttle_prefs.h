// Copyright 2024 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_THROTTLE_THROTTLE_PREFS_H_
#define CHROME_BROWSER_UI_WEBUI_THROTTLE_THROTTLE_PREFS_H_

#include "components/prefs/pref_registry_simple.h"

class PrefRegistrySimple;

namespace throttle_webui {
  namespace prefs {
     extern const char kNetworkThrottlingOffline[];
     extern const char kNetworkThrottlingLatency[];
     extern const char kNetworkThrottlingDownloadThroughput[];
     extern const char kNetworkThrottlingUploadThroughput[];
     extern const char kNetworkThrottlingPacketLoss[];
     extern const char kNetworkThrottlingPacketQueueLength[];
  }  // namespace prefs

  // Registers user preferences related to bookmarks.
  void RegisterProfilePrefs(PrefRegistrySimple* registry);
}

#endif  // CHROME_BROWSER_UI_WEBUI_THROTTLE_THROTTLE_PREFS_H_
