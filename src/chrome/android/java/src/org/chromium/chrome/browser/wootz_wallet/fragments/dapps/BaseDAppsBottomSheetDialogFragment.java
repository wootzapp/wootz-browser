/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet.fragments.dapps;

import android.app.Activity;

import org.chromium.wootz_wallet.mojom.WootzWalletService;
import org.chromium.wootz_wallet.mojom.JsonRpcService;
import org.chromium.wootz_wallet.mojom.KeyringService;
import org.chromium.chrome.browser.wootz_wallet.activities.WootzWalletBaseActivity;
import org.chromium.chrome.browser.wootz_wallet.fragments.WalletBottomSheetDialogFragment;

public class BaseDAppsBottomSheetDialogFragment extends WalletBottomSheetDialogFragment {
    public WootzWalletService getWootzWalletService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getWootzWalletService();
        }

        return null;
    }

    public KeyringService getKeyringService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getKeyringService();
        }

        return null;
    }

    public JsonRpcService getJsonRpcService() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletBaseActivity) {
            return ((WootzWalletBaseActivity) activity).getJsonRpcService();
        }

        return null;
    }
}
