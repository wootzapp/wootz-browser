// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PASSWORD_MANAGER_ANDROID_PASSWORD_MANAGER_ANDROID_UTIL_H_
#define CHROME_BROWSER_PASSWORD_MANAGER_ANDROID_PASSWORD_MANAGER_ANDROID_UTIL_H_

class PrefService;

namespace base {
class FilePath;
}  // namespace base

namespace password_manager_android_util {

// Checks that the GMS backend can be used, irrespective of whether for account
// or local passwords.
bool CanUseUPMBackend(bool is_pwd_sync_enabled, PrefService* pref_service);

// Called on startup to update the value of UsesSplitStoresAndUPMForLocal(),
// based on feature flags, minimum GmsCore version and other criteria.
// If switches::kSkipLocalUpmGmsCoreVersionCheckForTesting is added to the
// command-line, the GmsCore version check will be skipped.
void SetUsesSplitStoresAndUPMForLocal(PrefService* pref_service,
                                      const base::FilePath& login_db_directory);

}  // namespace password_manager_android_util

#endif  // CHROME_BROWSER_PASSWORD_MANAGER_ANDROID_PASSWORD_MANAGER_ANDROID_UTIL_H_
