// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "content/public/browser/content_privacy_prefs.h"
#include "components/prefs/pref_service.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "chrome/android/chrome_jni_headers/ContentPrivacyPreferencesBridge_jni.h"

static jboolean JNI_ContentPrivacyPreferencesBridge_IsContentPrivacyEnabled(JNIEnv* env) {
  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    return false;  // Default to disabled if no profile
  }
  
  PrefService* prefs = profile->GetPrefs();
  if (!prefs) {
    return false;  // Default to disabled if no prefs
  }
  
  // Get the content privacy enabled preference (defaults to false)
  return prefs->GetBoolean(content_privacy::prefs::kContentPrivacyEnabled);
} 