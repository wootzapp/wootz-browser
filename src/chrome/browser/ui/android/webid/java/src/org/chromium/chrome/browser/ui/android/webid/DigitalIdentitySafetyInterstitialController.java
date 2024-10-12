// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.android.webid;

import android.content.Context;
import android.content.res.Resources;

import org.chromium.base.Callback;
import org.chromium.base.ContextUtils;
import org.chromium.components.url_formatter.SchemeDisplay;
import org.chromium.components.url_formatter.UrlFormatter;
import org.chromium.ui.UiUtils;
import org.chromium.ui.modaldialog.DialogDismissalCause;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogProperties;
import org.chromium.ui.modaldialog.ModalDialogProperties.ButtonType;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.url.Origin;

/** Shows modal dialog asking user whether they want to share their identity with website. */
public class DigitalIdentitySafetyInterstitialController {
    private DigitalIdentitySafetyInterstitialController() {}

    public static void show(
            ModalDialogManager modalDialogManager,
            boolean isHighRisk,
            Origin origin,
            Callback<Integer> callback) {
        ModalDialogProperties.Controller controller =
                new ModalDialogProperties.Controller() {
                    @Override
                    public void onClick(PropertyModel model, @ButtonType int buttonType) {
                        if (buttonType == ButtonType.POSITIVE) {
                            modalDialogManager.dismissDialog(
                                    model, DialogDismissalCause.POSITIVE_BUTTON_CLICKED);
                        } else {
                            assert buttonType == ButtonType.NEGATIVE;
                            modalDialogManager.dismissDialog(
                                    model, DialogDismissalCause.NEGATIVE_BUTTON_CLICKED);
                        }
                    }

                    @Override
                    public void onDismiss(
                            PropertyModel model, @DialogDismissalCause int dismissalCause) {
                        callback.onResult(dismissalCause);
                    }
                };

        int bodyTextResourceId =
                isHighRisk
                        ? R.string.digital_identity_interstitial_high_risk_dialog_text
                        : R.string.digital_identity_interstitial_low_risk_dialog_text;
        int positiveButtonTextResourceId =
                isHighRisk
                        ? R.string.digital_identity_interstitial_high_risk_positive_button_text
                        : R.string.digital_identity_interstitial_low_risk_positive_button_text;
        @ModalDialogProperties.ButtonStyles
        int buttonStyles =
                isHighRisk
                        ? ModalDialogProperties.ButtonStyles.PRIMARY_OUTLINE_NEGATIVE_FILLED
                        : ModalDialogProperties.ButtonStyles.PRIMARY_FILLED_NEGATIVE_OUTLINE;

        Context context = ContextUtils.getApplicationContext();
        String bodyText =
                context.getString(
                        bodyTextResourceId,
                        UrlFormatter.formatOriginForSecurityDisplay(origin, SchemeDisplay.SHOW));

        Resources resources = context.getResources();
        PropertyModel.Builder dialogModelBuilder =
                new PropertyModel.Builder(ModalDialogProperties.ALL_KEYS)
                        .with(ModalDialogProperties.CONTROLLER, controller)
                        .with(
                                ModalDialogProperties.TITLE,
                                resources,
                                R.string.digital_identity_interstitial_dialog_title)
                        .with(ModalDialogProperties.MESSAGE_PARAGRAPH_1, bodyText)
                        .with(
                                ModalDialogProperties.POSITIVE_BUTTON_TEXT,
                                resources,
                                positiveButtonTextResourceId)
                        .with(
                                ModalDialogProperties.NEGATIVE_BUTTON_TEXT,
                                resources,
                                R.string.digital_identity_interstitial_negative_button_text)
                        .with(ModalDialogProperties.BUTTON_STYLES, buttonStyles)
                        .with(
                                ModalDialogProperties.BUTTON_TAP_PROTECTION_PERIOD_MS,
                                UiUtils.PROMPT_INPUT_PROTECTION_SHORT_DELAY_MS);
        modalDialogManager.showDialog(
                dialogModelBuilder.build(), ModalDialogManager.ModalDialogType.APP);
    }
}
