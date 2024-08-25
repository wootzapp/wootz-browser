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
import android.widget.TextView;

import org.chromium.chrome.R;

/**
 * Dialog that is displayed to ask user whether they want to remove the extension.
 */
public class ExtensionUninstallCustomScrollView
        extends ScrollView implements OnCheckedChangeListener {
    private TextView mSubtitleView;
    private CheckBox mCheckbox;

    private boolean mCheckboxChecked;

    public ExtensionUninstallCustomScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mCheckboxChecked = false;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mSubtitleView = findViewById(R.id.subtitle);
        mCheckbox = findViewById(R.id.checkbox);
    }

    void initialize(String extension_name,
            String windowTitle, String heading,
            boolean checkbox, String checkboxLabel) {

        if (checkbox) {
            mCheckbox.setVisibility(View.VISIBLE);
            mCheckbox.setText(checkboxLabel);
            mCheckbox.setChecked(false);
        }
        mCheckbox.setOnCheckedChangeListener(this);

        mSubtitleView.setText(heading);
    }

    protected boolean getcheckboxChecked() {
        return mCheckboxChecked;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        mCheckboxChecked = isChecked;
    }
}