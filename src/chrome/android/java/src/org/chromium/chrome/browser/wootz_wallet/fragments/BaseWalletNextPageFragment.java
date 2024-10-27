/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet.fragments;

import android.app.Activity;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import org.chromium.wootz_wallet.mojom.WootzWalletP3a;
import org.chromium.wootz_wallet.mojom.JsonRpcService;
import org.chromium.wootz_wallet.mojom.KeyringService;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.domain.KeyringModel;
import org.chromium.chrome.browser.app.domain.NetworkModel;
import org.chromium.chrome.browser.wootz_wallet.activities.WootzWalletActivity;
import org.chromium.chrome.browser.wootz_wallet.listeners.OnNextPage;
import org.chromium.chrome.browser.wootz_wallet.util.KeystoreHelper;
import org.chromium.chrome.browser.wootz_wallet.util.Utils;
import org.chromium.ui.widget.Toast;

import java.io.IOException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableEntryException;
import java.security.cert.CertificateException;
import java.util.concurrent.Executor;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
/**
 * Base Wootz Wallet fragment that performs a cast on the host activity to extract {@link
 * OnNextPage} interface used for basic navigation actions.
 */
public abstract class BaseWalletNextPageFragment extends Fragment {

    // Might be {@code null} when detached from the screen.
    @Nullable protected OnNextPage mOnNextPage;

    @Nullable private AnimationDrawable mAnimationDrawable;

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        try {
            mOnNextPage = (OnNextPage) context;
        } catch (ClassCastException e) {
            throw new ClassCastException("Host activity must implement OnNextPage interface.");
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mAnimationDrawable != null) {
            mAnimationDrawable.start();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mAnimationDrawable != null) {
            mAnimationDrawable.stop();
        }
    }

    @Override
    public void onDetach() {
        mOnNextPage = null;
        super.onDetach();
    }

    @Nullable
    protected NetworkModel getNetworkModel() {
        Activity activity = requireActivity();
        if (activity instanceof WootzWalletActivity) {
            return ((WootzWalletActivity) activity).getNetworkModel();
        }

        return null;
    }

    @Nullable
    protected KeyringModel getKeyringModel() {
        Activity activity = requireActivity();
        if (activity instanceof WootzWalletActivity) {
            return ((WootzWalletActivity) activity).getKeyringModel();
        }

        return null;
    }

    @Nullable
    protected KeyringService getKeyringService() {
        Activity activity = requireActivity();
        if (activity instanceof WootzWalletActivity) {
            return ((WootzWalletActivity) activity).getKeyringService();
        }

        return null;
    }

    @Nullable
    protected JsonRpcService getJsonRpcService() {
        Activity activity = requireActivity();
        if (activity instanceof WootzWalletActivity) {
            return ((WootzWalletActivity) activity).getJsonRpcService();
        }

        return null;
    }

    @Nullable
    protected WootzWalletP3a getWootzWalletP3A() {
        Activity activity = getActivity();
        if (activity instanceof WootzWalletActivity) {
            return ((WootzWalletActivity) activity).getWootzWalletP3A();
        }

        return null;
    }

    protected void setAnimatedBackground(@NonNull final View rootView) {
        mAnimationDrawable =
                (AnimationDrawable)
                        ContextCompat.getDrawable(
                                requireContext(), R.drawable.onboarding_gradient_animation);
        if (mAnimationDrawable != null) {
            rootView.setBackground(mAnimationDrawable);
            mAnimationDrawable.setEnterFadeDuration(10);
            mAnimationDrawable.setExitFadeDuration(5000);
        }
    }
}
