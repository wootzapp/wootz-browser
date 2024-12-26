// Copyright 2024 The WootzApp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid
package org.chromium.chrome.browser.ui.appmenu;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;

import com.google.android.material.bottomsheet.BottomSheetBehavior;
import com.google.android.material.bottomsheet.BottomSheetDialog;

import org.chromium.base.ContextUtils;
import org.chromium.base.version_info.VersionInfo;
import org.chromium.chrome.browser.extensions.ExtensionInfo;
import org.chromium.chrome.browser.extensions.Extensions;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.chrome.browser.ui.appmenu.internal.R;
import org.chromium.components.embedder_support.view.ContentView;
import org.chromium.components.thinwebview.ThinWebView;
import org.chromium.components.thinwebview.ThinWebViewConstraints;
import org.chromium.components.thinwebview.ThinWebViewFactory;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.WebContentsObserver;
import org.chromium.chrome.browser.content.WebContentsFactory;
import org.chromium.ui.base.IntentRequestTracker;
import org.chromium.ui.base.ViewAndroidDelegate;
import org.chromium.ui.base.WindowAndroid;

import java.util.ArrayList;

public class AppMenuExtensionOpener {
    private static final String TAG = "AppMenuExtensionOpener";
    
    private final Context mContext;
    private final WindowAndroid mWindowAndroid;
    private WebContents mCurrentWebContents;
    private static BottomSheetDialog mBottomSheetDialog;

    public AppMenuExtensionOpener(Context context, WindowAndroid windowAndroid) {
        mContext = context;
        mWindowAndroid = windowAndroid;
    }

    public void openExtension(String extensionId) {
        int index = findExtensionIndexById(extensionId);
        if (index == -1) {
            Log.e(TAG, "JANGID: Extension not found with ID: " + extensionId);
            return;
        }

        View webView = createWebView(index);
        if (webView == null) {
            Log.e(TAG, "JANGID: Failed to create WebView for extension");
            return;
        }

        showWebViewInBottomSheet(webView);
    }

    private View createWebView(int index) {
        try {
            Profile profile = ProfileManager.getLastUsedRegularProfile();
            mCurrentWebContents = WebContentsFactory.createWebContents(profile, true, false);
            ContentView contentView = ContentView.createContentView(mContext, null, mCurrentWebContents);

            mCurrentWebContents.setDelegates(
                VersionInfo.getProductVersion(),
                ViewAndroidDelegate.createBasicDelegate(contentView),
                contentView,
                mWindowAndroid,
                WebContents.createDefaultInternalsHolder());

            IntentRequestTracker intentRequestTracker = mWindowAndroid.getIntentRequestTracker();
            ThinWebView thinWebView = ThinWebViewFactory.create(
                mContext, new ThinWebViewConstraints(), intentRequestTracker);
            thinWebView.attachWebContents(mCurrentWebContents, contentView, null);
            
            // Adding deafult popup URL for SignMessage as this specific path is only for SignMessagePopup

            String popupUrl = Extensions.getExtensionsInfo().get(index).getPopupUrl();

            Log.d(TAG,"JANGID: popup URL" + popupUrl);

            Log.d(TAG,"JANGID: final popup URL" + popupUrl);
            
            mCurrentWebContents.getNavigationController().loadUrl(new LoadUrlParams(popupUrl));

            return thinWebView.getView();

        } catch (Exception e) {
            Log.e(TAG, "JANGID: Exception in createWebView", e);
            e.printStackTrace();
            return null;
        }
    }

    private void showWebViewInBottomSheet(View webView) {
        mBottomSheetDialog = new BottomSheetDialog(mContext, R.style.ExtensionsBottomSheetDialogTheme);
        View bottomSheetView = LayoutInflater.from(mContext).inflate(R.layout.extension_bottom_sheet_layout, null);
        
        FrameLayout webViewContainer = bottomSheetView.findViewById(R.id.web_view_container);
        webViewContainer.addView(webView);

        mBottomSheetDialog.setContentView(bottomSheetView);

        BottomSheetBehavior<View> behavior = BottomSheetBehavior.from((View) bottomSheetView.getParent());
        behavior.setState(BottomSheetBehavior.STATE_EXPANDED);

        mBottomSheetDialog.show();
    }

    private int findExtensionIndexById(String extensionId) {
        ArrayList<ExtensionInfo> extensions = Extensions.getExtensionsInfo();
        for (int i = 0; i < extensions.size(); i++) {
            if (extensions.get(i).getId().equals(extensionId)) {
                return i;
            }
        }
        return -1;
    }

    public static void closeBottomSheet() {
        if (mBottomSheetDialog != null && mBottomSheetDialog.isShowing()) {
            mBottomSheetDialog.dismiss();
        }
    }
}