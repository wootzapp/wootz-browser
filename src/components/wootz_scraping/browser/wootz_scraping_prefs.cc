/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_scraping/browser/wootz_scraping_prefs.h"

#include <string>
#include <utility>
#include <vector>

#include "base/values.h"
#include "components/wootz_scraping/browser/pref_names.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "components/p3a_utils/feature_usage.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/sync_preferences/pref_service_syncable.h"

namespace wootz_scraping {
namespace {

}  // namespace

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  // Twitter API credentials storage
  registry->RegisterDictionaryPref(kWootzScrapingTwitterAPICredentials);
}

}  // namespace wootz_scraping
