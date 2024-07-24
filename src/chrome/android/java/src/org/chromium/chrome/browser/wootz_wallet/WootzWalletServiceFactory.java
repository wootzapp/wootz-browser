/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.wootz_wallet.mojom.WootzWalletP3a;
import org.chromium.wootz_wallet.mojom.WootzWalletService;
import org.chromium.wootz_wallet.mojom.EthTxManagerProxy;
import org.chromium.wootz_wallet.mojom.JsonRpcService;
import org.chromium.wootz_wallet.mojom.KeyringService;
import org.chromium.wootz_wallet.mojom.SolanaTxManagerProxy;
import org.chromium.wootz_wallet.mojom.TxService;
import org.chromium.chrome.browser.wootz_wallet.util.Utils;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.mojo.bindings.ConnectionErrorHandler;
import org.chromium.mojo.bindings.Interface;
import org.chromium.mojo.bindings.Interface.Proxy.Handler;
import org.chromium.mojo.system.MessagePipeHandle;
import org.chromium.mojo.system.impl.CoreImpl;

@JNINamespace("chrome::android")
public class WootzWalletServiceFactory {
    private static final Object sLock = new Object();
    private static WootzWalletServiceFactory sInstance;

    public static WootzWalletServiceFactory getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new WootzWalletServiceFactory();
            }
        }
        return sInstance;
    }

    private WootzWalletServiceFactory() {}

    public WootzWalletService getWootzWalletService(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToWootzWalletService(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        WootzWalletService wootzWalletService = WootzWalletService.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) wootzWalletService).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return wootzWalletService;
    }

    public WootzWalletP3a getWootzWalletP3A(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToWootzWalletP3A(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        WootzWalletP3a wootzWalletP3A = WootzWalletP3a.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) wootzWalletP3A).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return wootzWalletP3A;
    }

    public JsonRpcService getJsonRpcService(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToJsonRpcService(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        JsonRpcService jsonRpcService = JsonRpcService.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) jsonRpcService).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return jsonRpcService;
    }

    public KeyringService getKeyringService(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // Always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToKeyringService(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        KeyringService keyringService = KeyringService.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) keyringService).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return keyringService;
    }

    public TxService getTxService(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle = WootzWalletServiceFactoryJni.get().getInterfaceToTxService(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        TxService txService = TxService.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) txService).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return txService;
    }

    public EthTxManagerProxy getEthTxManagerProxy(ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToEthTxManagerProxy(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        EthTxManagerProxy ethTxManagerProxy = EthTxManagerProxy.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) ethTxManagerProxy).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return ethTxManagerProxy;
    }

    public SolanaTxManagerProxy getSolanaTxManagerProxy(
            ConnectionErrorHandler connectionErrorHandler) {
        Profile profile = Utils.getProfile(false); // always use regular profile
        long nativeHandle =
                WootzWalletServiceFactoryJni.get().getInterfaceToSolanaTxManagerProxy(profile);
        MessagePipeHandle handle = wrapNativeHandle(nativeHandle);
        SolanaTxManagerProxy solanaTxManagerProxy =
                SolanaTxManagerProxy.MANAGER.attachProxy(handle, 0);
        Handler handler = ((Interface.Proxy) solanaTxManagerProxy).getProxyHandler();
        handler.setErrorHandler(connectionErrorHandler);

        return solanaTxManagerProxy;
    }

    private MessagePipeHandle wrapNativeHandle(long nativeHandle) {
        return CoreImpl.getInstance().acquireNativeHandle(nativeHandle).toMessagePipeHandle();
    }

    @NativeMethods
    interface Natives {
        long getInterfaceToWootzWalletService(Profile profile);

        long getInterfaceToWootzWalletP3A(Profile profile);

        long getInterfaceToJsonRpcService(Profile profile);

        long getInterfaceToKeyringService(Profile profile);

        long getInterfaceToTxService(Profile profile);

        long getInterfaceToEthTxManagerProxy(Profile profile);

        long getInterfaceToSolanaTxManagerProxy(Profile profile);
    }
}
