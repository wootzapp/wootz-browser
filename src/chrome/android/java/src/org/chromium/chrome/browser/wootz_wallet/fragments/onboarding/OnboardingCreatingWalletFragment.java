/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet.fragments.onboarding;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.chromium.base.task.PostTask;
import org.chromium.base.task.TaskTraits;
import org.chromium.wootz_wallet.mojom.WootzWalletP3a;
import org.chromium.wootz_wallet.mojom.JsonRpcService;
import org.chromium.wootz_wallet.mojom.NetworkInfo;
import org.chromium.wootz_wallet.mojom.OnboardingAction;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.domain.KeyringModel;
import org.chromium.chrome.browser.wootz_wallet.util.Utils;

import java.util.Set;

/** Onboarding fragment for Wootz Wallet which shows the spinner while wallet is created/restored */
public class OnboardingCreatingWalletFragment extends BaseOnboardingWalletFragment {

    private static final int NEXT_PAGE_DELAY_MS = 700;

    private boolean mAddTransitionDelay = true;

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_creating_wallet, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        setAnimatedBackground(view.findViewById(R.id.creating_wallet_root));
    }

    @Override
    public void onResume() {
        super.onResume();

        PostTask.postDelayedTask(
                TaskTraits.USER_BLOCKING, () -> mAddTransitionDelay = false, NEXT_PAGE_DELAY_MS);

        KeyringModel keyringModel = getKeyringModel();
        if (keyringModel != null) {
            // Check if a wallet is already present and skip if that's the case.
            keyringModel.isWalletCreated(
                    isCreated -> {
                        if (isCreated) {
                            goToNextPage();
                            return;
                        }
                        createWallet(keyringModel);
                    });
        }
    }

    private void createWallet(@NonNull final KeyringModel keyringModel) {
        WootzWalletP3a wootzWalletP3A = getWootzWalletP3A();
        JsonRpcService jsonRpcService = getJsonRpcService();

        if (jsonRpcService != null) {
            Set<NetworkInfo> availableNetworks = mOnboardingViewModel.getAvailableNetworks();
            Set<NetworkInfo> selectedNetworks = mOnboardingViewModel.getSelectedNetworks();
            keyringModel.createWallet(
                    mOnboardingViewModel.getPassword(),
                    availableNetworks,
                    selectedNetworks,
                    jsonRpcService,
                    recoveryPhrases -> {
                        if (wootzWalletP3A != null) {
                            wootzWalletP3A.reportOnboardingAction(OnboardingAction.RECOVERY_SETUP);
                        }

                        Utils.setCryptoOnboarding(false);
                        goToNextPage();
                    });
        }
    }

    private void goToNextPage() {
        // Go to the next page after wallet creation is done.
        if (mOnNextPage != null) {
            // Add small delay if the Wallet creation completes faster than {@code
            // NEXT_PAGE_DELAY_MS}.
            if (mAddTransitionDelay) {
                PostTask.postDelayedTask(
                        TaskTraits.USER_BLOCKING,
                        () -> mOnNextPage.incrementPages(1),
                        NEXT_PAGE_DELAY_MS);
            } else {
                mOnNextPage.incrementPages(1);
            }
        }
    }

    @Override
    protected boolean canBeClosed() {
        return false;
    }

    @Override
    protected boolean canNavigateBack() {
        return false;
    }
}
