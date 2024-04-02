// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/hello_wallet/hello_wallet_page_handler.h"

#include <utility>

#include "base/functional/bind.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"

HelloWalletPageHandler::HelloWalletPageHandler(
    mojo::PendingReceiver<solana_wallet::mojom::PageHandler> receiver,
    mojo::PendingRemote<solana_wallet::mojom::Page> page,
    Profile* profile)
    : profile_(profile),
      receiver_(this, std::move(receiver)),
      weak_ptr_factory_(this) {}

HelloWalletPageHandler::~HelloWalletPageHandler() = default;

void HelloWalletPageHandler::ShowApprovePanelUI() {
  
}

void HelloWalletPageHandler::ShowWalletBackupUI() {}
