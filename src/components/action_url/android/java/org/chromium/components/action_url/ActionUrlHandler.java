// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.action_url;

import android.util.Log;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.url.GURL;

@JNINamespace("action_url")
public class ActionUrlHandler {
    private long mNativePtr;

    @CalledByNative
    private ActionUrlHandler(long nativePtr) {
        mNativePtr = nativePtr;
    }

    @CalledByNative
    private void clearNativePtr() {
        mNativePtr = 0;
    }

    @CalledByNative
    private void getActionUrlForBlinkUrl(GURL blinkUrl, final long nativeCallback) {
        Log.i("Unfurling ::", "Blink URL is: " + blinkUrl.getSpec());

        ActionUrlFetchedCallback callback =
                (String actionUrl, String tag) -> {
                    if (mNativePtr != 0) {
                        ActionUrlHandlerJni.get().ActionUrlFetched(actionUrl, tag, nativeCallback);
                    }
                };

        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://rps.sendarcade.fun/api/actions/rps");
        // }
        // if(blinkUrl.getSpec().equals("https://example.com/")) {
        //     blinkUrl = new GURL("https://zk.tinys.pl/");
        // }
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://actions.buddy.link/treasury");
        // }
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://app.daovote.fun");
        // }
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://blinks.cribins.com");
        // }
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://quick-blinks.xyz/api/events/create");
        // }
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     blinkUrl = new GURL("https://xdares.catoff.xyz/api/actions/create-x-dares?clusterurl=mainnet");
        // }
        // if(blinkUrl.getSpec().equals("https://example.com/")) {
        //     blinkUrl = new GURL("https://neverhaveiever.catoff.xyz/api/actions/create-never-have-i-ever?clusterurl=mainnet");
        // }
        // if(blinkUrl.getSpec().equals("https://example.com/")) {
        //     blinkUrl = new GURL("https://drop.site/api/tweet/1805588148212424901");
        // }

        ActionUrlListFetcher.getInstance().getActionUrl(blinkUrl.getSpec(), callback);
        // Check the Global map to find the corresponding action url for blink url
        // String actionUrl = "";
        // if(blinkUrl.getSpec().equals("https://score.sendarcade.fun/")) {
        //     // actionUrl = "https://join.catoff.xyz/api/actions/create-challenge?clusterurl=mainnet&participationtype=1";
        //     // actionUrl = "https://pass.solana.id/api/actions/nft-mint";
        //     // actionUrl = "https://api.zk.tinys.pl/actions";
        //     actionUrl = "https://valorant.catoff.xyz/api/actions/create-valorant-challenge";
        // }

        // if(blinkUrl.getSpec().equals("https://example.com/")) {
        //     // actionUrl = "https://join.catoff.xyz/api/actions/create-challenge?clusterurl=mainnet&participationtype=1";
        //     // actionUrl = "https://pass.solana.id/api/actions/nft-mint";
        //     actionUrl = "https://api.zk.tinys.pl/actions";
        //     // retuactionUrl =rn "https://valorant.catoff.xyz/api/actions/create-valorant-challenge";
        // }

        // callback.onCompletion(actionUrl);
    }


    @NativeMethods
    interface Natives {
        void ActionUrlFetched(String actionUrl, String tag, long Callback);
    }
}
