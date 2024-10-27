/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.app.domain;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import org.chromium.base.Callbacks;
import org.chromium.wootz_wallet.mojom.AccountInfo;
import org.chromium.wootz_wallet.mojom.WootzWalletService;
import org.chromium.wootz_wallet.mojom.ByteArrayStringUnion;
import org.chromium.wootz_wallet.mojom.CoinType;
import org.chromium.wootz_wallet.mojom.JsonRpcService;
import org.chromium.wootz_wallet.mojom.KeyringService;
import org.chromium.wootz_wallet.mojom.KeyringServiceObserver;
import org.chromium.wootz_wallet.mojom.SignAllTransactionsRequest;
import org.chromium.wootz_wallet.mojom.SignMessageRequest;
import org.chromium.wootz_wallet.mojom.SignTransactionRequest;
import org.chromium.wootz_wallet.mojom.TransactionInfo;
import org.chromium.wootz_wallet.mojom.TransactionStatus;
import org.chromium.chrome.browser.wootz_wallet.activities.WootzWalletDAppsActivity;
import org.chromium.chrome.browser.wootz_wallet.model.WalletAccountCreationRequest;
import org.chromium.chrome.browser.wootz_wallet.util.PendingTxHelper;
import org.chromium.chrome.browser.wootz_wallet.util.Utils;
import org.chromium.mojo.system.MojoException;
import org.chromium.mojo.system.Pair;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class DappsModel implements KeyringServiceObserver {
    private JsonRpcService mJsonRpcService;
    private KeyringService mKeyringService;
    private WootzWalletService mWootzWalletService;
    private PendingTxHelper mPendingTxHelper;
    private final MutableLiveData<Boolean> _mWalletIconNotificationVisible =
            new MutableLiveData<>(false);
    private final Object mLock = new Object();
    private final MutableLiveData<WootzWalletDAppsActivity.ActivityType> _mProcessNextDAppsRequest =
            new MutableLiveData<>();
    private final MutableLiveData<List<SignTransactionRequest>> _mSignTxRequests;
    private final MutableLiveData<List<SignAllTransactionsRequest>> _mSignAllTxRequests;
    private final LiveData<List<SignTransactionRequest>> mSignTxRequests;
    private final LiveData<List<SignAllTransactionsRequest>> mSignAllTxRequests;
    private List<WalletAccountCreationRequest> mPendingWalletAccountCreationRequests;
    private MutableLiveData<WalletAccountCreationRequest> _mPendingWalletAccountCreationRequest;
    public LiveData<WalletAccountCreationRequest> mPendingWalletAccountCreationRequest;
    public final LiveData<Boolean> mWalletIconNotificationVisible = _mWalletIconNotificationVisible;
    public final LiveData<WootzWalletDAppsActivity.ActivityType> mProcessNextDAppsRequest =
            _mProcessNextDAppsRequest;

    public DappsModel(JsonRpcService jsonRpcService, WootzWalletService wootzWalletService,
            KeyringService keyringService, PendingTxHelper pendingTxHelper) {
        mWootzWalletService = wootzWalletService;
        mJsonRpcService = jsonRpcService;
        mKeyringService = keyringService;
        mPendingTxHelper = pendingTxHelper;
        _mSignTxRequests = new MutableLiveData<>(Collections.emptyList());
        mSignTxRequests = _mSignTxRequests;
        _mSignAllTxRequests = new MutableLiveData<>(Collections.emptyList());
        mSignAllTxRequests = _mSignAllTxRequests;
        _mPendingWalletAccountCreationRequest = new MutableLiveData<>();
        mPendingWalletAccountCreationRequest = _mPendingWalletAccountCreationRequest;
        mPendingWalletAccountCreationRequests = new ArrayList<>();
        mKeyringService.addObserver(this);
    }

    public void fetchAccountsForConnectionReq(@CoinType.EnumType int coinType,
            Callbacks.Callback1<Pair<AccountInfo, List<AccountInfo>>> callback) {
        if (coinType != CoinType.ETH && coinType != CoinType.SOL) {
            callback.call(new Pair<>(null, Collections.emptyList()));
            return;
        }

        mKeyringService.getAllAccounts(
                allAccounts -> {
                    List<AccountInfo> accounts =
                            Utils.filterAccountsByCoin(allAccounts.accounts, coinType);
                    callback.call(new Pair<>(allAccounts.ethDappSelectedAccount, accounts));
                });
    }

    public LiveData<List<SignTransactionRequest>> fetchSignTxRequest() {
        mWootzWalletService.getPendingSignTransactionRequests(requests -> {
            _mSignTxRequests.postValue(new ArrayList<>(Arrays.asList(requests)));
        });
        return mSignTxRequests;
    }

    public void signTxRequest(boolean isApproved, SignTransactionRequest signTransactionRequest) {
        mWootzWalletService.notifySignTransactionRequestProcessed(
                isApproved, signTransactionRequest.id, null, null);
        mWootzWalletService.getPendingSignTransactionRequests(requests -> {
            if (requests.length == 0) {
                _mProcessNextDAppsRequest.postValue(WootzWalletDAppsActivity.ActivityType.FINISH);
            } else {
                _mSignTxRequests.postValue(new ArrayList<>(Arrays.asList(requests)));
            }
        });
    }

    public LiveData<List<SignAllTransactionsRequest>> fetchSignAllTxRequest() {
        mWootzWalletService.getPendingSignAllTransactionsRequests(requests -> {
            _mSignAllTxRequests.postValue(new ArrayList<>(Arrays.asList(requests)));
        });
        return mSignAllTxRequests;
    }

    public void signAllTxRequest(boolean isApproved, SignAllTransactionsRequest request) {
        mWootzWalletService.notifySignAllTransactionsRequestProcessed(
                isApproved, request.id, null, null);
        mWootzWalletService.getPendingSignAllTransactionsRequests(requests -> {
            if (requests.length == 0) {
                _mProcessNextDAppsRequest.postValue(WootzWalletDAppsActivity.ActivityType.FINISH);
            } else {
                _mSignAllTxRequests.postValue(new ArrayList<>(Arrays.asList(requests)));
            }
        });
    }

    public void resetServices(JsonRpcService jsonRpcService,
            WootzWalletService wootzWalletService, PendingTxHelper pendingTxHelper) {
        synchronized (mLock) {
            mWootzWalletService = wootzWalletService;
            mJsonRpcService = jsonRpcService;
            mPendingTxHelper = pendingTxHelper;
        }
    }

    public void updateWalletBadgeVisibility() {
        synchronized (mLock) {
            updateWalletBadgeVisibilityInternal();
        }
    }

    public void clearDappsState() {
        _mProcessNextDAppsRequest.postValue(null);
    }

    public void processPublicEncryptionKey(String requestId, boolean isApproved) {
        synchronized (mLock) {
            if (mWootzWalletService == null) {
                return;
            }
            mWootzWalletService.notifyGetPublicKeyRequestProcessed(requestId, isApproved);
            mWootzWalletService.getPendingGetEncryptionPublicKeyRequests(requests -> {
                if (requests != null && requests.length > 0) {
                    _mProcessNextDAppsRequest.postValue(WootzWalletDAppsActivity.ActivityType
                                                                .GET_ENCRYPTION_PUBLIC_KEY_REQUEST);
                } else {
                    _mProcessNextDAppsRequest.postValue(
                            WootzWalletDAppsActivity.ActivityType.FINISH);
                }
            });
        }
    }

    public void processDecryptRequest(String requestId, boolean isApproved) {
        synchronized (mLock) {
            if (mWootzWalletService == null) {
                return;
            }
            mWootzWalletService.notifyDecryptRequestProcessed(requestId, isApproved);
            mWootzWalletService.getPendingDecryptRequests(requests -> {
                if (requests != null && requests.length > 0) {
                    _mProcessNextDAppsRequest.postValue(
                            WootzWalletDAppsActivity.ActivityType.DECRYPT_REQUEST);
                } else {
                    _mProcessNextDAppsRequest.postValue(
                            WootzWalletDAppsActivity.ActivityType.FINISH);
                }
            });
        }
    }

    public void setWalletBadgeVisible() {
        _mWalletIconNotificationVisible.setValue(true);
    }

    public void setWalletBadgeInvisible() {
        _mWalletIconNotificationVisible.setValue(false);
    }

    public void addAccountCreationRequest(@CoinType.EnumType int coinType) {
        Utils.removeIf(mPendingWalletAccountCreationRequests,
                request -> request.getCoinType() == coinType);
        WalletAccountCreationRequest request = new WalletAccountCreationRequest(coinType);
        mPendingWalletAccountCreationRequests.add(request);
        updatePendingAccountCreationRequest();
    }

    public void removeProcessedAccountCreationRequest(WalletAccountCreationRequest request) {
        if (request == null) return;
        Utils.removeIf(mPendingWalletAccountCreationRequests,
                input -> input.getCoinType() == request.getCoinType());
        updatePendingAccountCreationRequest();
    }

    public void showPendingAccountCreationRequest() {
        WalletAccountCreationRequest request = _mPendingWalletAccountCreationRequest.getValue();
        if (request != null) {
            _mPendingWalletAccountCreationRequest.postValue(request);
        } else if (!mPendingWalletAccountCreationRequests.isEmpty()) {
            _mPendingWalletAccountCreationRequest.postValue(
                    mPendingWalletAccountCreationRequests.get(0));
        }
    }

    public void notifySignMessageRequestProcessed(boolean isApproved, int id) {
        notifySignMessageRequestProcessed(isApproved, id, null, null);
    }
    public void notifySignMessageRequestProcessed(
            boolean isApproved, int id, ByteArrayStringUnion signature, String error) {
        mWootzWalletService.notifySignMessageRequestProcessed(isApproved, id, signature, error);
    }
    public void getPendingSignMessageRequests(Callbacks.Callback1<SignMessageRequest[]> callback) {
        mWootzWalletService.getPendingSignMessageRequests(callback::call);
    }

    private void updateWalletBadgeVisibilityInternal() {
        if (mWootzWalletService == null || mJsonRpcService == null || mPendingTxHelper == null) {
            return;
        }

        _mWalletIconNotificationVisible.setValue(false);

        mWootzWalletService.getPendingSignMessageRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mWootzWalletService.getPendingAddSuggestTokenRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mWootzWalletService.getPendingGetEncryptionPublicKeyRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mJsonRpcService.getPendingAddChainRequests(networks -> {
            if (networks != null && networks.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mJsonRpcService.getPendingSwitchChainRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mWootzWalletService.getPendingDecryptRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mWootzWalletService.getPendingSignTransactionRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        mWootzWalletService.getPendingSignAllTransactionsRequests(requests -> {
            if (requests != null && requests.length > 0) {
                setWalletBadgeVisible();
            }
        });
        for (TransactionInfo info : mPendingTxHelper.mTransactionInfoLd.getValue()) {
            if (info.txStatus == TransactionStatus.UNAPPROVED) {
                setWalletBadgeVisible();
                break;
            }
        }
    }

    private void updatePendingAccountCreationRequest() {
        if (mPendingWalletAccountCreationRequests.isEmpty()) {
            _mPendingWalletAccountCreationRequest.postValue(null);
        } else {
            _mPendingWalletAccountCreationRequest.postValue(
                    mPendingWalletAccountCreationRequests.get(0));
        }
    }

    @Override
    public void walletCreated() {}

    @Override
    public void walletRestored() {}

    @Override
    public void walletReset() {}

    @Override
    public void locked() {}

    @Override
    public void unlocked() {
        showPendingAccountCreationRequest();
    }

    @Override
    public void backedUp() {}

    @Override
    public void accountsChanged() {}

    @Override
    public void accountsAdded(AccountInfo[] addedAccounts) {}

    @Override
    public void autoLockMinutesChanged() {}

    @Override
    public void selectedWalletAccountChanged(AccountInfo accountInfo) {}

    @Override
    public void selectedDappAccountChanged(
            @CoinType.EnumType int coinType, AccountInfo accountInfo) {}

    @Override
    public void onConnectionError(MojoException e) {}

    @Override
    public void close() {}
}
