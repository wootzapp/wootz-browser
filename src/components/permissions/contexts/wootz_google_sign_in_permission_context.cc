// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "components/permissions/contexts/wootz_google_sign_in_permission_context.h"

#include <utility>

#include "components/content_settings/browser/page_specific_content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/permissions/permission_request_id.h"
#include "third_party/blink/public/mojom/permissions_policy/permissions_policy.mojom.h"

namespace permissions {

WootzGoogleSignInPermissionContext::WootzGoogleSignInPermissionContext(
    content::BrowserContext* browser_context)
    : PermissionContextBase(browser_context,
                            ContentSettingsType::WOOTZ_GOOGLE_SIGN_IN,
                            blink::mojom::PermissionsPolicyFeature::kNotFound) {
}

WootzGoogleSignInPermissionContext::~WootzGoogleSignInPermissionContext() =
    default;

bool WootzGoogleSignInPermissionContext::IsRestrictedToSecureOrigins() const {
  return false;
}
}  // namespace permissions
