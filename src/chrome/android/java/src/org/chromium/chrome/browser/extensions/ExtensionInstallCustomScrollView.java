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
import android.util.AttributeSet;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.Nullable;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.download.DownloadUtils;
import org.chromium.chrome.browser.download.settings.DownloadDirectoryAdapter;
import org.chromium.components.browser_ui.widget.text.AlertDialogEditText;

import java.io.File;
import java.util.StringJoiner;

/**
 * Dialog that is displayed to ask user where they want to download the file.
 */
public class ExtensionInstallCustomScrollView
        extends ScrollView implements OnCheckedChangeListener {
    private TextView mSubtitleView;
    private TextView mPermissionsHeading;
    private TextView mPermissions;
    private TextView mRetainedFilesHeading;
    private TextView mRetainedFiles;
    private TextView mRetainedDevicesHeading;
    private TextView mRetainedDeviceMessage;
    private CheckBox mWithholdPermission;

    private boolean mWithholdPermissionChecked;

    public ExtensionInstallCustomScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mWithholdPermissionChecked = false;
   }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mSubtitleView = findViewById(R.id.subtitle);
        mPermissionsHeading = findViewById(R.id.permissions_heading);
        mPermissions = findViewById(R.id.permissions);
        mRetainedFilesHeading = findViewById(R.id.retained_files_heading);
        mRetainedFiles = findViewById(R.id.retained_files);
        mRetainedDevicesHeading = findViewById(R.id.retained_devices_heading);
        mRetainedDeviceMessage = findViewById(R.id.retained_device_message);
        mWithholdPermission = findViewById(R.id.withhold_permission_checkbox);
    }

    void initialize(String title, String storeLink, String withholdPermissionsHeading,
            boolean shouldDisplayWithholdingUI, boolean shouldShowPermissions, int permissionCount,
            String[] permissions, String[] permissionsDetails, String permissionsHeading,
            String[] retainedFiles, String retainedFilesHeading,
            String[] retainedDeviceMessageStrings, String retainedDevicesHeading) {

        if (shouldDisplayWithholdingUI) {
            mWithholdPermission.setVisibility(View.VISIBLE);
            mWithholdPermission.setText(withholdPermissionsHeading);
            mWithholdPermission.setChecked(false);
        }
        mWithholdPermission.setOnCheckedChangeListener(this);

        mSubtitleView.setText(title);

        mPermissionsHeading.setText(permissionsHeading);
        if (shouldShowPermissions) {
            char bull = '\u2022';
            String newline = "";
            String indent = "    ";
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < permissionCount; ++i) {
                // Format strings
                sb.append(newline).append(bull).append(permissions[i]);
                newline = "\n";
                sb.append(newline).append(indent).append(permissionsDetails[i]);
            }
           mPermissions.setText(sb.toString());
            mPermissions.setVisibility(View.VISIBLE);
        }

        if (retainedFiles.length > 0) {
            String newline = "";
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < retainedFiles.length; ++i) {
                // Format strings
                sb.append(newline).append(retainedFiles[i]);
                newline = "\n";
            }
            mRetainedFiles.setText(sb.toString());
            mRetainedFiles.setVisibility(View.VISIBLE);
            mRetainedFilesHeading.setText(retainedFilesHeading);
            mRetainedFilesHeading.setVisibility(View.VISIBLE);
        }

        if (retainedDeviceMessageStrings.length > 0) {
            String newline = "";
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < retainedDeviceMessageStrings.length; ++i) {
                // Format strings
                sb.append(newline).append(retainedDeviceMessageStrings[i]);
                newline = "\n";
            }
            mRetainedDeviceMessage.setText(sb.toString());
            mRetainedDeviceMessage.setVisibility(View.VISIBLE);
            mRetainedDevicesHeading.setText(retainedDevicesHeading);
            mRetainedDevicesHeading.setVisibility(View.VISIBLE);
        }
    }

    protected boolean getWithholdPermissionChecked() {
        return mWithholdPermissionChecked;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        mWithholdPermissionChecked = isChecked;
    }
}