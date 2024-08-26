/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/profiles/wootz_renderer_updater.h"

#include <utility>

#include "base/check_is_test.h"
#include "base/functional/bind.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "chrome/common/wootz_renderer_configuration.mojom.h"
#include "components/wootz_wallet/browser/keyring_service.h"
#include "components/wootz_wallet/browser/pref_names.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/constants/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "extensions/buildflags/buildflags.h"
#include "ipc/ipc_channel_proxy.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/extension_registry.h"
#endif

WootzRendererUpdater::WootzRendererUpdater(
    Profile* profile,
    wootz_wallet::KeyringService* keyring_service,
    PrefService* local_state)
    : profile_(profile),
      keyring_service_(keyring_service),
      local_state_(local_state) {
  PrefService* pref_service = profile->GetPrefs();
  wootz_wallet_ethereum_provider_.Init(kDefaultEthereumWallet, pref_service);
  wootz_wallet_solana_provider_.Init(kDefaultSolanaWallet, pref_service);

  CheckActiveWallet();

  pref_change_registrar_.Init(pref_service);
  pref_change_registrar_.Add(
      kDefaultEthereumWallet,
      base::BindRepeating(&WootzRendererUpdater::UpdateAllRenderers,
                          base::Unretained(this)));
  pref_change_registrar_.Add(
      kDefaultSolanaWallet,
      base::BindRepeating(&WootzRendererUpdater::UpdateAllRenderers,
                          base::Unretained(this)));
 
  pref_change_registrar_.Add(
      kWootzWalletKeyrings,
      base::BindRepeating(
          &WootzRendererUpdater::CheckActiveWalletAndMaybeUpdateRenderers,
          base::Unretained(this)));

}

WootzRendererUpdater::~WootzRendererUpdater() = default;

void WootzRendererUpdater::InitializeRenderer(
    content::RenderProcessHost* render_process_host) {
  auto renderer_configuration = GetRendererConfiguration(render_process_host);
  Profile* profile =
      Profile::FromBrowserContext(render_process_host->GetBrowserContext());
  is_wallet_allowed_for_context_ = wootz_wallet::IsAllowedForContext(profile);
  UpdateRenderer(&renderer_configuration);
}

std::vector<mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>>
WootzRendererUpdater::GetRendererConfigurations() {
  std::vector<mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>>
      rv;
  for (content::RenderProcessHost::iterator it(
           content::RenderProcessHost::AllHostsIterator());
       !it.IsAtEnd(); it.Advance()) {
    Profile* renderer_profile =
        static_cast<Profile*>(it.GetCurrentValue()->GetBrowserContext());
    if (renderer_profile == profile_ ||
        renderer_profile->GetOriginalProfile() == profile_) {
      auto renderer_configuration =
          GetRendererConfiguration(it.GetCurrentValue());
      if (renderer_configuration) {
        rv.push_back(std::move(renderer_configuration));
      }
    }
  }
  return rv;
}

mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>
WootzRendererUpdater::GetRendererConfiguration(
    content::RenderProcessHost* render_process_host) {
  IPC::ChannelProxy* channel = render_process_host->GetChannel();
  if (!channel) {
    return mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>();
  }

  mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>
      renderer_configuration;
  channel->GetRemoteAssociatedInterface(&renderer_configuration);

  return renderer_configuration;
}

void WootzRendererUpdater::CheckActiveWalletAndMaybeUpdateRenderers() {
  if (CheckActiveWallet()) {
    UpdateAllRenderers();
  }
}

bool WootzRendererUpdater::CheckActiveWallet() {
  if (!keyring_service_) {
    return false;
  }
  bool is_wallet_created = keyring_service_->IsWalletCreatedSync();
  bool changed = is_wallet_created != is_wallet_created_;
  is_wallet_created_ = is_wallet_created;
  return changed;
}

void WootzRendererUpdater::UpdateAllRenderers() {
  auto renderer_configurations = GetRendererConfigurations();
  for (auto& renderer_configuration : renderer_configurations) {
    UpdateRenderer(&renderer_configuration);
  }
}

void WootzRendererUpdater::UpdateRenderer(
    mojo::AssociatedRemote<wootz::mojom::WootzRendererConfiguration>*
        renderer_configuration) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  extensions::ExtensionRegistry* registry =
      extensions::ExtensionRegistry::Get(profile_);
  bool has_installed_metamask =
      registry && registry->enabled_extensions().Contains(kMetamaskExtensionId);
#else
  bool has_installed_metamask = false;
#endif

  bool should_ignore_wootz_wallet_for_eth =
      !is_wallet_created_ || has_installed_metamask;

  auto default_ethereum_wallet =
      static_cast<wootz_wallet::mojom::DefaultWallet>(
          wootz_wallet_ethereum_provider_.GetValue());
  bool install_window_wootz_ethereum_provider =
      is_wallet_allowed_for_context_ &&
      default_ethereum_wallet != wootz_wallet::mojom::DefaultWallet::None;
  bool install_window_ethereum_provider =
      ((default_ethereum_wallet ==
            wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension &&
        !should_ignore_wootz_wallet_for_eth) ||
       default_ethereum_wallet ==
           wootz_wallet::mojom::DefaultWallet::WootzWallet) &&
      is_wallet_allowed_for_context_;
  bool allow_overwrite_window_ethereum_provider =
      default_ethereum_wallet ==
      wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension;

  auto default_solana_wallet = static_cast<wootz_wallet::mojom::DefaultWallet>(
      wootz_wallet_solana_provider_.GetValue());
  bool wootz_use_native_solana_wallet =
      (default_solana_wallet ==
           wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension ||
       default_solana_wallet ==
           wootz_wallet::mojom::DefaultWallet::WootzWallet) &&
      is_wallet_allowed_for_context_;
  bool allow_overwrite_window_solana_provider =
      default_solana_wallet ==
      wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension;


  (*renderer_configuration)
      ->SetConfiguration(wootz::mojom::DynamicParams::New(
          install_window_wootz_ethereum_provider,
          install_window_ethereum_provider,
          allow_overwrite_window_ethereum_provider,
          wootz_use_native_solana_wallet,
          allow_overwrite_window_solana_provider, false,
          false,false, false));
}
