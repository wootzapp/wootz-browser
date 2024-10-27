/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_H_
#define CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_H_

#include <vector>

#include "base/memory/raw_ptr.h"
#include "chrome/common/wootz_renderer_configuration.mojom-forward.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_member.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/remote.h"

class Profile;

namespace wootz_wallet {
class KeyringService;
}

namespace content {
class RenderProcessHost;
}

class WootzRendererUpdater : public KeyedService {
 public:
  WootzRendererUpdater(Profile* profile,
                       wootz_wallet::KeyringService* keyring_service,
                       PrefService* local_state);
  WootzRendererUpdater(const WootzRendererUpdater&) = delete;
  WootzRendererUpdater& operator=(const WootzRendererUpdater&) = delete;
  ~WootzRendererUpdater() override;

  // Initialize a newly-started renderer process.
  void InitializeRenderer(content::RenderProcessHost* render_process_host);

 private:
  std::vector<mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>>
  GetRendererConfigurations();

  mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>
  GetRendererConfiguration(content::RenderProcessHost* render_process_host);

  // Update renderers if wallet keyring has been initialized
  void CheckActiveWalletAndMaybeUpdateRenderers();

  // Update active wallet bool, returns true if status has changed
  bool CheckActiveWallet();

  // Update all renderers due to a configuration change.
  void UpdateAllRenderers();

  // Update the given renderer due to a configuration change.
  void UpdateRenderer(
      mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>*
          renderer_configuration);

  raw_ptr<Profile> profile_ = nullptr;
  raw_ptr<wootz_wallet::KeyringService> keyring_service_ = nullptr;
  raw_ptr<PrefService> local_state_ = nullptr;
  PrefChangeRegistrar pref_change_registrar_;
  PrefChangeRegistrar local_state_change_registrar_;

  // Prefs that we sync to the renderers.
  IntegerPrefMember wootz_wallet_ethereum_provider_;
  IntegerPrefMember wootz_wallet_solana_provider_;
    
  bool is_wallet_allowed_for_context_ = false;
  bool is_wallet_created_ = false;
};

#endif  // CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_H_
