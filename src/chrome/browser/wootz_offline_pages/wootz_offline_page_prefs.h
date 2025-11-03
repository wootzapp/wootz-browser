// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_

class PrefRegistrySimple;

namespace wootz_offline_pages {
namespace prefs {

// Boolean pref to enable/disable offline browsing feature.
// When enabled, pages are automatically saved as MHTML and served offline.
inline constexpr char kOfflineBrowsingEnabled[] = 
    "wootz.offline_pages.enabled";

// Registers preferences for offline browsing.
void RegisterProfilePrefs(PrefRegistrySimple* registry);

}  // namespace prefs
}  // namespace wootz_offline_pages

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_

