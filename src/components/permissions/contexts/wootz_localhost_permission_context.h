// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_LOCALHOST_PERMISSION_CONTEXT_H_
#define COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_LOCALHOST_PERMISSION_CONTEXT_H_

#include "components/permissions/permission_context_base.h"
#include "content/public/browser/browser_context.h"

namespace permissions {

class WootzLocalhostPermissionContext : public PermissionContextBase {
 public:
  // using PermissionContextBase::RequestPermission;
  explicit WootzLocalhostPermissionContext(
      content::BrowserContext* browser_context);
  ~WootzLocalhostPermissionContext() override;

  WootzLocalhostPermissionContext(const WootzLocalhostPermissionContext&) =
      delete;
  WootzLocalhostPermissionContext& operator=(
      const WootzLocalhostPermissionContext&) = delete;

 private:
  bool IsRestrictedToSecureOrigins() const override;
};

}  // namespace permissions

#endif  // COMPONENTS_PERMISSIONS_CONTEXTS_WOOTZ_LOCALHOST_PERMISSION_CONTEXT_H_
