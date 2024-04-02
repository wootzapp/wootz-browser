// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "components/autofill/ios/common/features.h"

// Features that are exlusive to iOS go here in alphabetical order.

// Controls whether to dynamically load the address input fields in the save
// flow and settings based on the country value.
// TODO(crbug.com/1482269): Remove once launched.
BASE_FEATURE(kAutofillDynamicallyLoadsFieldsForAddressInput,
             "AutofillDynamicallyLoadsFieldsForAddressInput",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Controls whether to use the isolated content world instead of the page
// content world for the Autofill JS feature scripts.
// TODO(crbug.com/1175793) Remove once the isolated content world is launched
// for Autofill.
BASE_FEATURE(kAutofillIsolatedWorldForJavascriptIos,
             "AutofillIsolatedWorldForJavascriptIos",
             base::FEATURE_DISABLED_BY_DEFAULT);
