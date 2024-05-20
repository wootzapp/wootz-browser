// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_PAYMENTS_FEATURES_H_
#define COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_PAYMENTS_FEATURES_H_

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "build/build_config.h"

namespace autofill {
namespace features {

// All features in alphabetical order.
BASE_DECLARE_FEATURE(kAutofillEnableAndroidNKeyForFidoAuthentication);
BASE_DECLARE_FEATURE(kAutofillEnableCardArtImage);
BASE_DECLARE_FEATURE(kAutofillEnableCardArtServerSideStretching);
BASE_DECLARE_FEATURE(kAutofillEnableCardBenefitsForAmericanExpress);
BASE_DECLARE_FEATURE(kAutofillEnableCardBenefitsForCapitalOne);
BASE_DECLARE_FEATURE(kAutofillEnableCardBenefitsSync);
BASE_DECLARE_FEATURE(kAutofillEnableCardProductName);
BASE_DECLARE_FEATURE(kAutofillEnableCvcStorageAndFilling);
BASE_DECLARE_FEATURE(kAutofillEnableFIDOProgressDialog);
BASE_DECLARE_FEATURE(kAutofillEnableFpanRiskBasedAuthentication);

#if BUILDFLAG(IS_ANDROID)
BASE_DECLARE_FEATURE(kAutofillEnableLocalIban);
#endif

BASE_DECLARE_FEATURE(kAutofillEnableMerchantDomainInUnmaskCardRequest);
BASE_DECLARE_FEATURE(kAutofillEnableMovingGPayLogoToTheRightOnDesktop);
BASE_DECLARE_FEATURE(kAutofillEnableMovingGPayLogoToTheRightOnClank);
BASE_DECLARE_FEATURE(kAutofillEnableNewCardArtAndNetworkImages);
BASE_DECLARE_FEATURE(kAutofillEnableOffersInClankKeyboardAccessory);
BASE_DECLARE_FEATURE(kAutofillEnablePrefetchingRiskDataForRetrieval);
BASE_DECLARE_FEATURE(kAutofillEnableRemadeDownstreamMetrics);
BASE_DECLARE_FEATURE(kAutofillEnableSaveCardLoadingAndConfirmation);
BASE_DECLARE_FEATURE(kAutofillEnableSaveCardLocalSaveFallback);
BASE_DECLARE_FEATURE(kAutofillEnableServerIban);
BASE_DECLARE_FEATURE(kAutofillEnableStickyManualFallbackForCards);

#if BUILDFLAG(IS_ANDROID)
BASE_DECLARE_FEATURE(kAutofillEnableSyncingOfPixBankAccounts);
#endif

BASE_DECLARE_FEATURE(kAutofillEnableVcn3dsAuthentication);
BASE_DECLARE_FEATURE(kAutofillEnableVcnEnrollLoadingAndConfirmation);
BASE_DECLARE_FEATURE(kAutofillEnableVcnGrayOutForMerchantOptOut);
BASE_DECLARE_FEATURE(kAutofillEnableVirtualCardEnrollMetricsLogger);
BASE_DECLARE_FEATURE(kAutofillEnableVerveCardSupport);
BASE_DECLARE_FEATURE(kAutofillEnableVirtualCardMetadata);
BASE_DECLARE_FEATURE(kAutofillParseVcnCardOnFileStandaloneCvcFields);
#if BUILDFLAG(IS_ANDROID)
BASE_DECLARE_FEATURE(kAutofillSkipAndroidBottomSheetForIban);
#endif
BASE_DECLARE_FEATURE(kAutofillUpdateChromeSettingsLinkToGPayWeb);
BASE_DECLARE_FEATURE(kAutofillUpstream);

#if BUILDFLAG(IS_IOS)
BASE_DECLARE_FEATURE(kAutofillUseTwoDotsForLastFourDigits);
BASE_DECLARE_FEATURE(kAutofillEnableVirtualCards);
#endif

// Return whether a [No thanks] button and new messaging is shown in the save
// card bubbles. This will be called only on desktop platforms.
bool ShouldShowImprovedUserConsentForCreditCardSave();

}  // namespace features
}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_COMMON_AUTOFILL_PAYMENTS_FEATURES_H_
