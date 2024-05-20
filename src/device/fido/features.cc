// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/features.h"

#include <vector>

#include "base/feature_list.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"

namespace device {

// Flags defined in this file should have a comment above them that either
// marks them as permanent flags, or specifies what lifecycle stage they're at.
//
// Permanent flags are those that we'll keep around indefinitely because
// they're useful for testing, debugging, etc. These should be commented with
//    // Permanent flag
//
// Standard flags progress through a lifecycle and are eliminated at the end of
// it. The comments above them should be one of the following:
//    // Not yet enabled by default.
//    // Enabled in M123. Remove in or after M126.
//
// Every release or so we should cleanup and delete flags which have been
// default-enabled for long enough, based on the removal milestone in their
// comment.

#if BUILDFLAG(IS_WIN)
// Permanent flag
BASE_FEATURE(kWebAuthUseNativeWinApi,
             "WebAuthenticationUseNativeWinApi",
             base::FEATURE_ENABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_WIN)

// Permanent flag
BASE_FEATURE(kWebAuthCableExtensionAnywhere,
             "WebAuthenticationCableExtensionAnywhere",
             base::FEATURE_DISABLED_BY_DEFAULT);

BASE_FEATURE(kWebAuthnGoogleCorpRemoteDesktopClientPrivilege,
             "WebAuthenticationGoogleCorpRemoteDesktopClientPrivilege",
             base::FEATURE_ENABLED_BY_DEFAULT);

#if BUILDFLAG(IS_ANDROID)
// Added in M114. Not yet enabled by default.
BASE_FEATURE(kWebAuthnAndroidCredMan,
             "WebAuthenticationAndroidCredMan",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_ANDROID)

// Added in M115. Remove in or after M118
BASE_FEATURE(kWebAuthnHybridLinkWithoutNotifications,
             "WebAuthenticationHybridLinkWithoutNotifications",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enabled in M118. Remove in or after M121.
BASE_FEATURE(kWebAuthnICloudKeychainForGoogle,
             "WebAuthenticationICloudKeychainForGoogle",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enabled in M118. Remove in or after M121.
BASE_FEATURE(kWebAuthnICloudKeychainForActiveWithDrive,
             "WebAuthenticationICloudKeychainForActiveWithDrive",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Not yet enabled by default.
BASE_FEATURE(kWebAuthnICloudKeychainForActiveWithoutDrive,
             "WebAuthenticationICloudKeychainForActiveWithoutDrive",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enabled in M118. Remove in or after M121.
BASE_FEATURE(kWebAuthnICloudKeychainForInactiveWithDrive,
             "WebAuthenticationICloudKeychainForInactiveWithDrive",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Not yet enabled by default.
BASE_FEATURE(kWebAuthnICloudKeychainForInactiveWithoutDrive,
             "WebAuthenticationICloudKeychainForInactiveWithoutDrive",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enabled in M117. Remove in or after M120.
BASE_FEATURE(kWebAuthnLinkingExperimentation,
             "WebAuthenticationLinkingExperimentation",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Not yet enabled by default.
BASE_FEATURE(kWebAuthnEnclaveAuthenticator,
             "WebAuthenticationEnclaveAuthenticator",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Not yet enabled by default.
const base::FeatureParam<bool> kWebAuthnGpmPin{
    &kWebAuthnEnclaveAuthenticator, kWebAuthnGpmPinFeatureParameterName,
    /*default_value=*/false};

// Enabled in M118 on all platforms except ChromeOS. Enabled on M121 for
// ChromeOS. Remove in or after M124.
BASE_FEATURE(kWebAuthnFilterGooglePasskeys,
             "WebAuthenticationFilterGooglePasskeys",
             base::FEATURE_ENABLED_BY_DEFAULT);

#if BUILDFLAG(IS_CHROMEOS)
// Not yet enabled by default.
BASE_FEATURE(kChromeOsPasskeys,
             "WebAuthenticationCrosPasskeys",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif

// Enabled in M120. Remove in or after M123.
BASE_FEATURE(kWebAuthnAccessibleTimeouts,
             "WebAuthenticationAccessibleTimeouts",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Not yet enabled by default.
BASE_FEATURE(kWebAuthnRelatedOrigin,
             "WebAuthenticationRelatedOrigin",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enabled in M122. Remove in or after M125.
BASE_FEATURE(kWebAuthnChromeImplementedInvariant,
             "WebAuthenticationChromeImplementedInvariant",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enabled in M122. Remove in or after M125.
BASE_FEATURE(kAllowExtensionsToSetWebAuthnRpIds,
             "AllowExtensionsToSetWebAuthnRpIds",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Default enabled in M122. Remove in or after M125.
BASE_FEATURE(kWebAuthnAndroidFidoJson,
             "WebAuthenticationAndroidFidoJson",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Default enabled in M123. Remove in or after M126.
BASE_FEATURE(kWebAuthnPreferVirtualPlatformAuthenticator,
             "WebAuthenticationPreferVirtualPlatformAuthenticator",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Deprecation flag.
// Default disabled in M125. Remove in or after M128.
BASE_FEATURE(kWebAuthnEnableAndroidCableAuthenticator,
             "WebAuthenticationEnableAndroidCableAuthenticator",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Development flag. Must not be enabled by default once
// kWebAuthnEnclaveAuthenticator is enabled.
BASE_FEATURE(kWebAuthnUseInsecureSoftwareUnexportableKeys,
             "WebAuthenticationUseInsecureSoftwareUnexportableKeys",
#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC)
             base::FEATURE_DISABLED_BY_DEFAULT);
#else
             base::FEATURE_ENABLED_BY_DEFAULT);
#endif

// Default enabled in M126. Remove in or after M129.
BASE_FEATURE(kWebAuthnCredProtectWin10BugWorkaround,
             "WebAuthenticationCredProtectWin10BugWorkaround",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Default enabled in M126. Remove in or after M129.
BASE_FEATURE(kWebAuthnICloudRecoveryKey,
             "WebAuthenticationICloudRecoveryKey",
             base::FEATURE_ENABLED_BY_DEFAULT);

}  // namespace device
