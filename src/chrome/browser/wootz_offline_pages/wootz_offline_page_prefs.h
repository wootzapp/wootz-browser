// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_

class PrefRegistrySimple;

namespace wootz_offline_pages {
namespace prefs {

// Boolean pref to enable/disable automatic saving of pages.
// When enabled, pages are automatically saved as MHTML while browsing.
inline constexpr char kOfflineBrowsingEnabled[] = 
    "wootz.offline_pages.enabled";

// Boolean pref to enable/disable auto-opening of saved offline pages.
// When enabled, saved offline pages are automatically served when navigating
// to a URL that has been previously saved.
inline constexpr char kAutoOpenOfflinePages[] = 
    "wootz.offline_pages.auto_open";

// Registers preferences for offline browsing.
void RegisterProfilePrefs(PrefRegistrySimple* registry);

}  // namespace prefs
}  // namespace wootz_offline_pages

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_PREFS_H_

