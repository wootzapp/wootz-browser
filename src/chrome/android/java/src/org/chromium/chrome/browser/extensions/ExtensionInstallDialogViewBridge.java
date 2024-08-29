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

import org.jni_zero.NativeMethods;
import org.jni_zero.CalledByNative;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.download.dialogs.DownloadLocationCustomView;
import org.chromium.chrome.R;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.modaldialog.DialogDismissalCause;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogProperties;
import org.chromium.ui.modelutil.PropertyModel;
import java.io.File;


/**
* Helper class to handle communication between extension install dialog and native.
*/

public class ExtensionInstallDialogViewBridge implements ModalDialogProperties.Controller {
    private long mNativeExtensionInstallDialogViewBridge;
    private PropertyModel mDialogModel;
    private ExtensionInstallCustomScrollView mCustomView;
    private ModalDialogManager mModalDialogManager;
    private Context mContext;

    private ExtensionInstallDialogViewBridge(long nativeExtensionInstallDialogViewBridge) {
        mNativeExtensionInstallDialogViewBridge = nativeExtensionInstallDialogViewBridge;
    }

    @CalledByNative
    public static ExtensionInstallDialogViewBridge create(long nativeExtensionInstallDialogView) {
        return new ExtensionInstallDialogViewBridge(nativeExtensionInstallDialogView);
    }

    @CalledByNative
    private void destroy() {
        mNativeExtensionInstallDialogViewBridge = 0;
        if (mModalDialogManager != null) {
            mModalDialogManager.dismissDialog(
                    mDialogModel, DialogDismissalCause.DISMISSED_BY_NATIVE);
        }
    }

    @CalledByNative
    public void showDialog(WindowAndroid windowAndroid,
            String title, String storeLink, String withholdPermissionsHeading,
            boolean shouldDisplayWithholdingUI, boolean shouldShowPermissions, int permissionCount,
            String[] permissions, String[] permissionsDetails, String permissionsHeading,
            String[] retainedFiles, String retainedFilesHeading,
            String[] retainedDeviceMessageStrings, String retainedDevicesHeading) {
        ChromeActivity activity = (ChromeActivity) windowAndroid.getActivity().get();
        // If the activity has gone away, just clean up the native pointer.
        if (activity == null) {
            onDismiss(null, DialogDismissalCause.ACTIVITY_DESTROYED);
            return;
        }

        // Check list of permissions match their length. If mismatch, dismiss with cancel
        if (permissions.length != permissionCount || permissionsDetails.length != permissionCount) {
            onDismiss(null, DialogDismissalCause.NEGATIVE_BUTTON_CLICKED);
            return;
        }

        mModalDialogManager = activity.getModalDialogManager();
        mContext = activity;

        // Already showing the dialog.
        if (mDialogModel != null) return;

        // Actually show the dialog.
       mCustomView = (ExtensionInstallCustomScrollView) LayoutInflater.from(mContext).inflate(
               R.layout.extension_install_dialog, null);
        mCustomView.initialize(title, storeLink, withholdPermissionsHeading,
                shouldDisplayWithholdingUI, shouldShowPermissions, permissionCount,
                permissions, permissionsDetails, permissionsHeading,
                retainedFiles, retainedFilesHeading,
                retainedDeviceMessageStrings, retainedDevicesHeading);

        Resources resources = mContext.getResources();
        mDialogModel =
                new PropertyModel.Builder(ModalDialogProperties.ALL_KEYS)
                        .with(ModalDialogProperties.CONTROLLER, this)
                        .with(ModalDialogProperties.TITLE, title)
                        .with(ModalDialogProperties.CUSTOM_VIEW, mCustomView)
                        .with(ModalDialogProperties.POSITIVE_BUTTON_TEXT, resources,
                                R.string.extension_install_prompt_ok_button)
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
                accept((mCustomView != null && mCustomView.getWithholdPermissionChecked()));
                break;
            default:
                cancel();
                break;
        }
        mDialogModel = null;
        mCustomView = null;
    }

    private void accept(boolean withholdPermissionsCheckboxChecked) {
        if (mNativeExtensionInstallDialogViewBridge != 0) {
            ExtensionInstallDialogViewBridgeJni.get().Accept(
                    mNativeExtensionInstallDialogViewBridge, ExtensionInstallDialogViewBridge.this,
                    withholdPermissionsCheckboxChecked);
        }
    }

    private void cancel() {
        if (mNativeExtensionInstallDialogViewBridge != 0) {
            ExtensionInstallDialogViewBridgeJni.get().Cancel(
                    mNativeExtensionInstallDialogViewBridge, ExtensionInstallDialogViewBridge.this);
        }
    }

    @NativeMethods
    interface Natives {
        void Cancel(
                long nativeExtensionInstallDialogView, ExtensionInstallDialogViewBridge caller);
        void Accept(
                long nativeExtensionInstallDialogView, ExtensionInstallDialogViewBridge caller,
                boolean withholdPermissionsCheckboxChecked);
    }
}