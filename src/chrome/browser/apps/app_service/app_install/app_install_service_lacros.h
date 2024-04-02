// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_APPS_APP_SERVICE_APP_INSTALL_APP_INSTALL_SERVICE_LACROS_H_
#define CHROME_BROWSER_APPS_APP_SERVICE_APP_INSTALL_APP_INSTALL_SERVICE_LACROS_H_

#include "base/memory/raw_ref.h"
#include "chrome/browser/apps/app_service/app_install/app_install_service.h"

namespace apps {

class PackageId;

class AppInstallServiceLacros : public AppInstallService {
 public:
  AppInstallServiceLacros();
  ~AppInstallServiceLacros() override;

  // AppInstallService:
  void InstallApp(AppInstallSurface surface,
                  PackageId package_id,
                  base::OnceClosure callback) override;
};

}  // namespace apps

#endif  // CHROME_BROWSER_APPS_APP_SERVICE_APP_INSTALL_APP_INSTALL_SERVICE_LACROS_H_
