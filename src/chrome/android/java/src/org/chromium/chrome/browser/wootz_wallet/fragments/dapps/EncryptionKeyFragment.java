/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet.fragments.dapps;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.fragment.app.Fragment;

import org.chromium.base.Log;
import org.chromium.wootz_wallet.mojom.AccountId;
import org.chromium.wootz_wallet.mojom.AccountInfo;
import org.chromium.wootz_wallet.mojom.CoinType;
import org.chromium.wootz_wallet.mojom.DecryptRequest;
import org.chromium.wootz_wallet.mojom.GetEncryptionPublicKeyRequest;
import org.chromium.wootz_wallet.mojom.NetworkInfo;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.app.domain.WalletModel;
import org.chromium.chrome.browser.wootz_wallet.activities.WootzWalletDAppsActivity;
import org.chromium.chrome.browser.wootz_wallet.util.AndroidUtils;
import org.chromium.chrome.browser.wootz_wallet.util.Utils;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class EncryptionKeyFragment extends Fragment implements View.OnClickListener {
    private static final String TAG = "ConnectAccount";

    private static final String ACTIVITY_TYPE = "param1";
    private ImageView mAccountImage;
    private TextView mAccountName;
    private TextView mTvMessage;
    private TextView mTvMessageDesc;
    private TextView mTvMessageDecrypt;
    private TextView mTveTldPlusOne;
    // TODO(apaymyshev): network doesn't make sense for supported requests.
    private TextView mNetworkName;
    private Button mBtCancel;
    private Button mBtProvideAllow;
    private ExecutorService mExecutor;
    private Handler mHandler;
    private WootzWalletDAppsActivity.ActivityType mActivityType;
    private WalletModel mWalletModel;
    private GetEncryptionPublicKeyRequest mEncryptionPublicKeyRequest;
    private DecryptRequest mDecryptRequest;

    /**
     * Factory method
     *
     * @param activityType to define the type of public-encryption-key or decrypt-request.
     * @return A new instance of fragment EncryptionKeyFragment.
     */
    public static EncryptionKeyFragment newInstance(
            WootzWalletDAppsActivity.ActivityType activityType) {
        EncryptionKeyFragment fragment = new EncryptionKeyFragment();
        Bundle args = new Bundle();
        args.putSerializable(ACTIVITY_TYPE, activityType);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mExecutor = Executors.newSingleThreadExecutor();
        mHandler = new Handler(Looper.getMainLooper());
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            mWalletModel = activity.getWalletModel();
            Log.e("WOOTZAPP ANKITIVAN", "getWalletModel: " + mWalletModel);
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "onCreate " + e);
        }
    }

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        assert getArguments() != null;
        mActivityType = (WootzWalletDAppsActivity.ActivityType) getArguments().getSerializable(
                ACTIVITY_TYPE);
        View view = inflater.inflate(R.layout.fragment_encryption_key, container, false);
        mAccountImage = view.findViewById(R.id.fragment_encryption_msg_cv_iv_account);
        mAccountName = view.findViewById(R.id.fragment_encryption_msg_tv_account_name);
        mTvMessageDesc = view.findViewById(R.id.fragment_encryption_msg_desc);
        mNetworkName = view.findViewById(R.id.fragment_encryption_msg_tv_network_name);
        mTvMessage = view.findViewById(R.id.fragment_encryption_msg_tv_message);
        mTveTldPlusOne = view.findViewById(R.id.fragment_encryption_msg_tv_eTldPlusOne);
        mTvMessageDecrypt = view.findViewById(R.id.fragment_encryption_tv_decrypt);

        mBtCancel = view.findViewById(R.id.fragment_encryption_msg_btn_cancel);
        // Title: "provide" for public key request and "allow" for decrypt request
        mBtProvideAllow = view.findViewById(R.id.fragment_encryption_msg_btn_provide_allow);
        mBtProvideAllow.setOnClickListener(this);
        mBtCancel.setOnClickListener(this);
        initState();

        return view;
    }

    private void updateAccount(AccountId accountId) {
        assert (accountId.coin == CoinType.ETH);
        mWalletModel.getKeyringModel().mAllAccountsInfo.observe(
                getViewLifecycleOwner(), allAccounts -> {
                    AccountInfo accountInfo = Utils.findAccount(allAccounts.accounts, accountId);
                    if (accountInfo != null) {
                        assert (accountInfo.address != null);
                        Utils.setBlockiesBitmapResourceFromAccount(
                                mExecutor, mHandler, mAccountImage, accountInfo, true);
                        String accountText = accountInfo.name + "\n" + accountInfo.address;
                        mAccountName.setText(accountText);
                    }
                });
    }

    private void initState() {
        if (mWalletModel == null) {
            return;
        }
        Log.e("WOOTZAPP ANKIT", "initState: mWalletModel " + mWalletModel);
        mWalletModel.getCryptoModel().getNetworkModel().mPairChainAndNetwork.observe(
                getViewLifecycleOwner(), chainIdAndInfosPair -> {
                    String chainId = chainIdAndInfosPair.first;
                    List<NetworkInfo> wootzNetworks = chainIdAndInfosPair.second;
                    NetworkInfo network = Utils.getNetworkInfoByChainId(chainId, wootzNetworks);
                    if (!TextUtils.isEmpty(chainId) && wootzNetworks.size() > 0) {
                        mNetworkName.setText(network.chainName);
                    }
                });

        if (mActivityType
                == WootzWalletDAppsActivity.ActivityType.GET_ENCRYPTION_PUBLIC_KEY_REQUEST) {
            mWalletModel.getCryptoModel().getPublicEncryptionRequest(encryptionPublicKeyRequest -> {
                if (encryptionPublicKeyRequest != null) {
                    mEncryptionPublicKeyRequest = encryptionPublicKeyRequest;
                    updateAccount(mEncryptionPublicKeyRequest.accountId);
                    String formattedeTLD = String.format(
                            getString(R.string.wootz_wallet_provide_encryption_key_description),
                            Utils.geteTldHtmlString(encryptionPublicKeyRequest.originInfo));
                    mTvMessageDesc.setText(AndroidUtils.formatHTML(formattedeTLD));
                }
            });
        } else if (mActivityType == WootzWalletDAppsActivity.ActivityType.DECRYPT_REQUEST) {
            mWalletModel.getCryptoModel().getDecryptMessageRequest(decryptRequest -> {
                mDecryptRequest = decryptRequest;
                updateAccount(mDecryptRequest.accountId);
                mTveTldPlusOne.setVisibility(View.VISIBLE);
                mTvMessageDecrypt.setVisibility(View.VISIBLE);
                mTveTldPlusOne.setText(Utils.geteTldSpanned(mDecryptRequest.originInfo));
                mTvMessage.setText(R.string.wootz_wallet_read_encrypted_message_title);
                mBtProvideAllow.setText(R.string.wootz_wallet_read_encrypted_message_button);
                mTvMessageDecrypt.setOnClickListener(v -> {
                    mTvMessageDecrypt.setVisibility(View.GONE);
                    mTvMessageDesc.setText(mDecryptRequest.unsafeMessage);
                });
            });
        }
    }

    @Override
    public void onClick(View v) {
        if (mActivityType
                == WootzWalletDAppsActivity.ActivityType.GET_ENCRYPTION_PUBLIC_KEY_REQUEST) {
            mWalletModel.getDappsModel().processPublicEncryptionKey(
                    mEncryptionPublicKeyRequest.requestId, isPositiveActionTriggered(v));
        } else if (mActivityType == WootzWalletDAppsActivity.ActivityType.DECRYPT_REQUEST) {
            mWalletModel.getDappsModel().processDecryptRequest(
                    mDecryptRequest.requestId, isPositiveActionTriggered(v));
        }
    }

    private boolean isPositiveActionTriggered(View v) {
        return v.getId() == R.id.fragment_encryption_msg_btn_provide_allow;
    }
}
