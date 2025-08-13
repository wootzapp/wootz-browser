// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/upload_blocking_prefs.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/values.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"

namespace content {

namespace upload_blocking_prefs {

// Pref name constants
const char kBlockedUploadDomains[] = "upload_blocking.blocked_domains";

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterListPref(kBlockedUploadDomains);
}

}  // namespace upload_blocking_prefs
}  // namespace content
