/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_INSTALLER_H_
#define COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_INSTALLER_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/values.h"
#include "components/wootz_component_updater/browser/wootz_component.h"
#include "components/component_updater/component_installer.h"
#include "components/update_client/update_client.h"

using wootz_component_updater::WootzComponent;

namespace wootz {

class WootzComponentInstallerPolicy
    : public component_updater::ComponentInstallerPolicy {
 public:
  explicit WootzComponentInstallerPolicy(
      const std::string& name,
      const std::string& base64_public_key,
      WootzComponent::ReadyCallback ready_callback);

  WootzComponentInstallerPolicy(const WootzComponentInstallerPolicy&) = delete;
  WootzComponentInstallerPolicy& operator=(
      const WootzComponentInstallerPolicy&) = delete;

  ~WootzComponentInstallerPolicy() override;

 private:
  // The following methods override ComponentInstallerPolicy
  bool VerifyInstallation(const base::Value::Dict& manifest,
                          const base::FilePath& install_dir) const override;
  bool SupportsGroupPolicyEnabledComponentUpdates() const override;
  bool RequiresNetworkEncryption() const override;
  update_client::CrxInstaller::Result OnCustomInstall(
      const base::Value::Dict& manifest,
      const base::FilePath& install_dir) override;
  void OnCustomUninstall() override;
  void ComponentReady(const base::Version& version,
                      const base::FilePath& install_dir,
                      base::Value::Dict manifest) override;
  base::FilePath GetRelativeInstallDir() const override;
  void GetHash(std::vector<uint8_t>* hash) const override;
  std::string GetName() const override;
  update_client::InstallerAttributes GetInstallerAttributes() const override;
  bool IsWootzComponent() const override;

  std::string name_;
  std::string base64_public_key_;
  std::string public_key_;
  WootzComponent::ReadyCallback ready_callback_;
};

void RegisterComponent(component_updater::ComponentUpdateService* cus,
                       const std::string& name,
                       const std::string& base64_public_key,
                       base::OnceClosure registered_callback,
                       WootzComponent::ReadyCallback ready_callback);

}  // namespace wootz

#endif  // COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_INSTALLER_H_
