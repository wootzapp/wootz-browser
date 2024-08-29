// Copyright 2021 The Ungoogled Chromium Authors. All rights reserved.
//
// This file is part of Ungoogled Chromium Android.
//
// Ungoogled Chromium Android is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or any later version.
//
// Ungoogled Chromium Android is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Ungoogled Chromium Android.  If not,
// see <https://www.gnu.org/licenses/>.

package org.chromium.chrome.browser.extensions;

import android.content.Context;
import android.content.res.Resources;
import android.view.LayoutInflater;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.R;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.modaldialog.DialogDismissalCause;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogProperties;
import org.chromium.ui.modelutil.PropertyModel;


/**
* Helper class to handle communication between extension uninstall dialog and native.
*/

public class ExtensionUninstallDialogBridge implements ModalDialogProperties.Controller {
    private long mNativeExtensionUninstallDialogDelegateView;
    private PropertyModel mDialogModel;
    private ExtensionUninstallCustomScrollView mCustomView;
    private ModalDialogManager mModalDialogManager;
    private Context mContext;

    private ExtensionUninstallDialogBridge(long nativeExtensionUninstallDialogDelegateView) {
        mNativeExtensionUninstallDialogDelegateView = nativeExtensionUninstallDialogDelegateView;
    }

    @CalledByNative
    public static ExtensionUninstallDialogBridge create(
            long nativeExtensionUninstallDialogDelegateView) {
        return new ExtensionUninstallDialogBridge(nativeExtensionUninstallDialogDelegateView);
    }

    @CalledByNative
    private void destroy() {
        mNativeExtensionUninstallDialogDelegateView = 0;
        if (mModalDialogManager != null) {
            mModalDialogManager.dismissDialog(
                    mDialogModel, DialogDismissalCause.DISMISSED_BY_NATIVE);
        }
    }

    @CalledByNative
    public void show(WindowAndroid windowAndroid,
            String extension_name, String windowTitle, String heading,
            boolean checkbox, String checkboxLabel) {
        ChromeActivity activity = (ChromeActivity) windowAndroid.getActivity().get();
        // If the activity has gone away, just clean up the native pointer.
        if (activity == null) {
            onDismiss(null, DialogDismissalCause.ACTIVITY_DESTROYED);
            return;
        }

        mModalDialogManager = activity.getModalDialogManager();
        mContext = activity;

        // Already showing the dialog.
        if (mDialogModel != null) return;

        // Actually show the dialog.
        mCustomView = (ExtensionUninstallCustomScrollView) LayoutInflater.from(mContext).inflate(
                R.layout.extension_uninstall_dialog, null);
        mCustomView.initialize(extension_name, windowTitle, heading,
                checkbox, checkboxLabel);

       Resources resources = mContext.getResources();
       mDialogModel =
                new PropertyModel.Builder(ModalDialogProperties.ALL_KEYS)
                        .with(ModalDialogProperties.CONTROLLER, this)
                        .with(ModalDialogProperties.TITLE, windowTitle)
                        .with(ModalDialogProperties.CUSTOM_VIEW, mCustomView)
                        .with(ModalDialogProperties.POSITIVE_BUTTON_TEXT, resources,
                                R.string.extension_prompt_uninstall_button)
                        .with(ModalDialogProperties.NEGATIVE_BUTTON_TEXT, resources,
                                R.string.cancel)
                        .build();

        mModalDialogManager.showDialog(mDialogModel, ModalDialogManager.ModalDialogType.APP);
    }

    @Override
    public void onClick(PropertyModel model, int buttonType) {
        switch (buttonType) {
            case ModalDialogProperties.ButtonType.POSITIVE:
                mModalDialogManager.dismissDialog(
                        model, DialogDismissalCause.POSITIVE_BUTTON_CLICKED);
                break;
            case ModalDialogProperties.ButtonType.NEGATIVE:
                mModalDialogManager.dismissDialog(
                        model, DialogDismissalCause.NEGATIVE_BUTTON_CLICKED);
                break;
            default:
        }
    }

    @Override
    public void onDismiss(PropertyModel model, int dismissalCause) {
        switch (dismissalCause) {
            case DialogDismissalCause.POSITIVE_BUTTON_CLICKED:
                accept((mCustomView != null && mCustomView.getcheckboxChecked()));
                break;
            default:
               cancel();
                break;
        }
        mDialogModel = null;
        mCustomView = null;
    }

    private void accept(boolean checkboxChecked) {
        if (mNativeExtensionUninstallDialogDelegateView != 0) {
            ExtensionUninstallDialogBridgeJni.get().Accept(
                    mNativeExtensionUninstallDialogDelegateView,
                    ExtensionUninstallDialogBridge.this,
                    checkboxChecked);
        }
    }

    private void cancel() {
        if (mNativeExtensionUninstallDialogDelegateView != 0) {
            ExtensionUninstallDialogBridgeJni.get().Cancel(
                    mNativeExtensionUninstallDialogDelegateView,
                    ExtensionUninstallDialogBridge.this);
        }
    }

    @NativeMethods
    interface Natives {
        void Cancel(
                long nativeExtensionUninstallDialogDelegateView,
                ExtensionUninstallDialogBridge caller);
        void Accept(
                long nativeExtensionUninstallDialogDelegateView,
                ExtensionUninstallDialogBridge caller,
                boolean checkboxChecked);
    }
}