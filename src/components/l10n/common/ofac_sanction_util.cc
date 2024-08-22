/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/l10n/common/ofac_sanction_util.h"

#include <string_view>

#include "base/containers/contains.h"
#include "base/strings/string_util.h"
#include "components/l10n/common/ofac_sanctioned_iso_3166_1_country_code_constants.h"
#include "components/l10n/common/ofac_sanctioned_un_m49_code_constants.h"

namespace wootz_l10n {

bool IsISOCountryCodeOFACSanctioned(const std::string_view country_code) {
  return base::Contains(kOFACSactionedISO31661CountryCodes,
                        base::ToUpperASCII(country_code));
}

bool IsUNM49CodeOFACSanctioned(const std::string_view code) {
  return base::Contains(kOFACSactionedUnM49Codes, code);
}

}  // namespace wootz_l10n
