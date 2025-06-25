// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.settings;

import android.os.Bundle;
import androidx.fragment.app.Fragment;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.util.Log;

import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.R;
import org.chromium.components.url_formatter.UrlFormatter;

/**
 * Provides the Java-UI for editing AdBlock preferences.
 */
public class AdBlockEditor extends Fragment implements TextWatcher {
    private EditText mAdBlockFiltersUrlEdit;
    private Button mSaveButton;
    private Button mResetButton;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("AdBlock", "AdBlockEditor onCreateView called");
        getActivity().setTitle(R.string.options_adblock_edit_title);

        View v = inflater.inflate(R.layout.adblock_editor, container, false);
        View scrollView = v.findViewById(R.id.scroll_view);
        scrollView.getViewTreeObserver().addOnScrollChangedListener(
                SettingsUtils.getShowShadowOnScrollListener(v, v.findViewById(R.id.shadow)));
        mAdBlockFiltersUrlEdit = (EditText) v.findViewById(R.id.adblock_url_edit);
        String currentUrl = ChromeFeatureList.getAdBlockFiltersURL();
        Log.i("AdBlock", "Current AdBlock filters URL: " + currentUrl);
        mAdBlockFiltersUrlEdit.setText(currentUrl);
        mAdBlockFiltersUrlEdit.addTextChangedListener(this);
        mAdBlockFiltersUrlEdit.requestFocus();

        initializeSaveCancelResetButtons(v);
        return v;
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
    }

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        Log.i("AdBlock", "URL text changed to: " + s);
        mSaveButton.setEnabled(s.length() != 0);
        mResetButton.setEnabled(true);
    }

    @Override
    public void afterTextChanged(Editable s) {
    }

    private void initializeSaveCancelResetButtons(View v) {
        mResetButton = (Button) v.findViewById(R.id.adblock_reset);
        mResetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String defaultUrl = ChromeFeatureList.getAdBlockFiltersURL();
                Log.i("AdBlock", "Reset clicked, restoring default URL: " + defaultUrl);
                mAdBlockFiltersUrlEdit.setText(defaultUrl);
            }
        });

        mSaveButton = (Button) v.findViewById(R.id.adblock_save);
        mSaveButton.setEnabled(false);
        mSaveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String newUrl = UrlFormatter.fixupUrl(mAdBlockFiltersUrlEdit.getText().toString()).getSpec();
                Log.i("AdBlock", "Save clicked, setting new URL: " + newUrl);
                ChromeFeatureList.setAdBlockFiltersURL(newUrl);
                getActivity().finish();
            }
        });

        Button button = (Button) v.findViewById(R.id.adblock_cancel);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i("AdBlock", "Cancel clicked");
                getActivity().finish();
            }
        });
    }
}