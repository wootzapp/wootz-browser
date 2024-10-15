/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/l10n/common/localization_util.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/base/resource/resource_bundle.h"

namespace wootz_l10n {

std::u16string GetLocalizedResourceUTF16String(const int resource_id) {
  const std::string resource =
      ui::ResourceBundle::GetSharedInstance().LoadLocalizedResourceString(
          resource_id);

  return base::UTF8ToUTF16(resource);
}

}  // namespace wootz_l10n
