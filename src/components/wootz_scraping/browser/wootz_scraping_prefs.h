/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_PREFS_H_
#define COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_PREFS_H_

// class PrefService;
// class PrefRegistrySimple;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace wootz_scraping {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace wootz_scraping

#endif  // COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_PREFS_H_
