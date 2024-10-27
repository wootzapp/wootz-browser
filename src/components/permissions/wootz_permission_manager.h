/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_PERMISSIONS_WOOTZ_PERMISSION_MANAGER_H_
#define COMPONENTS_PERMISSIONS_WOOTZ_PERMISSION_MANAGER_H_

#include <vector>

#include "components/permissions/permission_manager.h"

namespace permissions {

class WootzPermissionManager : public PermissionManager {
 public:
  WootzPermissionManager(content::BrowserContext* browser_context,
                         PermissionContextMap permission_contexts);
  ~WootzPermissionManager() override = default;

  WootzPermissionManager(const WootzPermissionManager&) = delete;
  WootzPermissionManager& operator=(const WootzPermissionManager&) = delete;

  void Shutdown() override;

  void RequestPermissionsForOrigin(
      const std::vector<blink::PermissionType>& permissions,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      bool user_gesture,
      base::OnceCallback<
          void(const std::vector<blink::mojom::PermissionStatus>&)> callback);

  blink::mojom::PermissionStatus GetPermissionStatusForOrigin(
      blink::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin) override;
};

}  // namespace permissions

#endif  // COMPONENTS_PERMISSIONS_WOOTZ_PERMISSION_MANAGER_H_
