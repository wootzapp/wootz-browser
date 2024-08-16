/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet.listeners;

import androidx.annotation.NonNull;

import org.chromium.chrome.browser.wootz_wallet.model.AccountSelectorItemModel;

public interface AccountSelectorItemListener {
    void onAccountClick(@NonNull final AccountSelectorItemModel accountSelectorItemModel);
}
