/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.settings;

import android.content.Context;
import android.content.DialogInterface;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.Preference;

import org.chromium.wootz_wallet.mojom.TxService;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.wootz_wallet.WootzWalletServiceFactory;
import org.chromium.mojo.bindings.ConnectionErrorHandler;
import org.chromium.mojo.system.MojoException;
import org.chromium.ui.KeyboardVisibilityDelegate;

/**
 * The preference used to reset transaction and nonce history in Wootz Wallet.
 */
public class WootzWalletResetTxHistoryAndNoncePreference
        extends Preference implements Preference.OnPreferenceClickListener, ConnectionErrorHandler {
    private String TAG = "WootzWalletResetTxHistoryAndNoncePreference";

    private TxService mTxService;
    private final String mConfirmationPhrase;

    public WootzWalletResetTxHistoryAndNoncePreference(Context context, AttributeSet attrs) {
        super(context, attrs);

        setOnPreferenceClickListener(this);
        mConfirmationPhrase = getContext().getResources().getString(
                R.string.wootz_wallet_reset_settings_confirmation_phrase);
        initTxService();
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        showWootzWalletResetTxHistoryAndNonceDialog();
        return true;
    }

    @Override
    public void onDetached() {
        super.onDetached();
        if (mTxService != null) {
            mTxService.close();
        }
    }

    private void showWootzWalletResetTxHistoryAndNonceDialog() {
        LayoutInflater inflater =
                (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view =
                inflater.inflate(R.layout.wootz_wallet_preference_confirmation_text_layout, null);
        TextView textView =
                view.findViewById(R.id.wootz_wallet_preference_confirmation_dialog_tv_message);
        textView.setText(getContext().getResources().getString(
                R.string.wootz_wallet_clear_tx_and_nonce_dialog_confirmation, mConfirmationPhrase));
        final EditText input =
                view.findViewById(R.id.wootz_wallet_preference_confirmation_dialog_edittext);

        DialogInterface.OnClickListener onClickListener = (dialog, button) -> {
            if (button == AlertDialog.BUTTON_POSITIVE) {
                mTxService.reset();
            } else {
                dialog.dismiss();
            }
        };

        AlertDialog.Builder alert =
                new AlertDialog.Builder(getContext(), R.style.ThemeOverlay_BrowserUI_AlertDialog);
        AlertDialog alertDialog =
                alert.setTitle(R.string.wootz_wallet_clear_tx_and_nonce_setting_title)
                        .setView(view)
                        .setPositiveButton(R.string.wootz_wallet_confirm_text, onClickListener)
                        .setNegativeButton(R.string.cancel, onClickListener)
                        .create();
        alertDialog.getDelegate().setHandleNativeActionModesEnabled(false);
        alertDialog.setOnShowListener(new DialogInterface.OnShowListener() {
            @Override
            public void onShow(DialogInterface dialog) {
                KeyboardVisibilityDelegate.getInstance().showKeyboard(input);
            }
        });
        alertDialog.show();
        final Button okButton = alertDialog.getButton(AlertDialog.BUTTON_POSITIVE);
        okButton.setEnabled(false);

        input.addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {}

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                // Disable ok button if input is invalid
                String inputText = s.toString().trim();

                okButton.setEnabled(TextUtils.equals(inputText, mConfirmationPhrase));
            }
        });
    }

    @Override
    public void onConnectionError(MojoException e) {
        mTxService.close();
        mTxService = null;
        initTxService();
    }

    private void initTxService() {
        if (mTxService != null) {
            return;
        }

        mTxService = WootzWalletServiceFactory.getInstance().getTxService(this);
    }
}
