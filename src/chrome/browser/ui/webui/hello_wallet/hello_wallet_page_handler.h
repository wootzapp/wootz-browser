// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_solana_wallet_solana_wallet_PAGE_HANDLER_IMPL_H_
#define CHROME_BROWSER_UI_WEBUI_solana_wallet_solana_wallet_PAGE_HANDLER_IMPL_H_

#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/solana_wallet/common/solana_wallet.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

class Profile;

class HelloWalletPageHandler : public solana_wallet::mojom::PageHandler {
 public:
  HelloWalletPageHandler(
      mojo::PendingReceiver<solana_wallet::mojom::PageHandler> receiver,
      mojo::PendingRemote<solana_wallet::mojom::Page> page,
      Profile* profile);
  ~HelloWalletPageHandler() override;

  HelloWalletPageHandler(const HelloWalletPageHandler&) = delete;
  HelloWalletPageHandler& operator=(const HelloWalletPageHandler&) = delete;

  // solana_wallet::mojom::PageHandler:
  void ShowApprovePanelUI() override;
  void ShowWalletBackupUI() override;

 private:
  raw_ptr<Profile> profile_ = nullptr;  // NOT OWNED
  mojo::Receiver<solana_wallet::mojom::PageHandler> receiver_;
  base::WeakPtrFactory<HelloWalletPageHandler> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_UI_WEBUI_solana_wallet_solana_wallet_PAGE_HANDLER_IMPL_H_
