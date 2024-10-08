// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_GOOGLE_SIGN_IN_PERMISSION_CONTEXT_H_
#define COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_GOOGLE_SIGN_IN_PERMISSION_CONTEXT_H_

#include "components/permissions/permission_context_base.h"
#include "content/public/browser/browser_context.h"

namespace permissions {

class WootzGoogleSignInPermissionContext : public PermissionContextBase {
 public:
  // using PermissionContextBase::RequestPermission;
  explicit WootzGoogleSignInPermissionContext(
      content::BrowserContext* browser_context);
  ~WootzGoogleSignInPermissionContext() override;

  WootzGoogleSignInPermissionContext(
      const WootzGoogleSignInPermissionContext&) = delete;
  WootzGoogleSignInPermissionContext& operator=(
      const WootzGoogleSignInPermissionContext&) = delete;

 private:
  bool IsRestrictedToSecureOrigins() const override;
};

}  // namespace permissions

#endif  // COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_GOOGLE_SIGN_IN_PERMISSION_CONTEXT_H_
