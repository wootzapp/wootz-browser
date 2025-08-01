// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.download.dialogs;

import android.content.Context;

import androidx.annotation.IntDef;
import androidx.core.content.res.ResourcesCompat;

import org.chromium.base.Callback;
import org.chromium.base.metrics.RecordHistogram;
import org.chromium.chrome.browser.download.R;
import org.chromium.components.browser_ui.util.DownloadUtils;
import org.chromium.ui.UiUtils;
import org.chromium.ui.modaldialog.DialogDismissalCause;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogProperties;
import org.chromium.ui.modelutil.PropertyModel;

import android.view.View;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.graphics.Typeface;
import android.view.Gravity;

/**
 * Dialog for confirming that user want to download a dangerous file, using the
 * default model dialog
 * from ModalDialogManager.
 */
public class DangerousDownloadDialog {
        /**
         * Events related to the dangerous download dialog, used for UMA reporting.
         * These values are persisted to logs. Entries should not be renumbered and
         * numeric values should never be reused.
         */
        @IntDef({
                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_SHOW,
                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CONFIRM,
                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CANCEL,
                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_DISMISS
        })
        private @interface DangerousDownloadDialogEvent {
                int DANGEROUS_DOWNLOAD_DIALOG_SHOW = 0;
                int DANGEROUS_DOWNLOAD_DIALOG_CONFIRM = 1;
                int DANGEROUS_DOWNLOAD_DIALOG_CANCEL = 2;
                int DANGEROUS_DOWNLOAD_DIALOG_DISMISS = 3;

                int COUNT = 4;
        }

        public DangerousDownloadDialog() {
        }

        /**
         * Called to show a warning dialog for dangerous download.
         * 
         * @param context            Context for showing the dialog.
         * @param modalDialogManager Manager for managing the modal dialog.
         * @param fileName           Name of the download file.
         * @param totalBytes         Total bytes of the file.
         * @param iconId             Icon ID of the warning dialog.
         * @param callback           Callback to run when confirming the download, true
         *                           for accept the download,
         *                           false otherwise.
         */
        public void show(
                        Context context,
                        ModalDialogManager modalDialogManager,
                        String fileName,
                        long totalBytes,
                        int iconId,
                        Callback<Boolean> callback) {
                var resources = context.getResources();
                String message = totalBytes > 0
                                ? resources.getString(R.string.dangerous_download_dialog_text, fileName)
                                : resources.getString(
                                                R.string.dangerous_download_dialog_text_with_size,
                                                fileName,
                                                DownloadUtils.getStringForBytes(context, totalBytes));

                var controller = new ModalDialogProperties.Controller() {
                        @Override
                        public void onClick(PropertyModel model, int buttonType) {
                                boolean acceptDownload = buttonType == ModalDialogProperties.ButtonType.POSITIVE;
                                if (callback != null) {
                                        callback.onResult(acceptDownload);
                                }
                                modalDialogManager.dismissDialog(
                                                model,
                                                acceptDownload
                                                                ? DialogDismissalCause.POSITIVE_BUTTON_CLICKED
                                                                : DialogDismissalCause.NEGATIVE_BUTTON_CLICKED);
                                recordDangerousDownloadDialogEvent(
                                                acceptDownload
                                                                ? DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CONFIRM
                                                                : DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CANCEL);
                        }

                        @Override
                        public void onDismiss(PropertyModel model, int dismissalCause) {
                                if (dismissalCause != DialogDismissalCause.POSITIVE_BUTTON_CLICKED
                                                && dismissalCause != DialogDismissalCause.NEGATIVE_BUTTON_CLICKED) {
                                        if (callback != null)
                                                callback.onResult(false);
                                        recordDangerousDownloadDialogEvent(
                                                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_DISMISS);
                                }
                        }
                };
                PropertyModel propertyModel = new PropertyModel.Builder(ModalDialogProperties.ALL_KEYS)
                                .with(ModalDialogProperties.CONTROLLER, controller)
                                .with(
                                                ModalDialogProperties.TITLE,
                                                resources.getString(R.string.dangerous_download_dialog_title))
                                .with(ModalDialogProperties.MESSAGE_PARAGRAPH_1, message)
                                .with(
                                                ModalDialogProperties.POSITIVE_BUTTON_TEXT,
                                                resources.getString(
                                                                R.string.dangerous_download_dialog_confirm_text))
                                .with(
                                                ModalDialogProperties.NEGATIVE_BUTTON_TEXT,
                                                resources.getString(R.string.cancel))
                                .with(
                                                ModalDialogProperties.TITLE_ICON,
                                                ResourcesCompat.getDrawable(resources, iconId, context.getTheme()))
                                .with(
                                                ModalDialogProperties.BUTTON_STYLES,
                                                ModalDialogProperties.ButtonStyles.PRIMARY_OUTLINE_NEGATIVE_OUTLINE)
                                .with(
                                                ModalDialogProperties.BUTTON_TAP_PROTECTION_PERIOD_MS,
                                                UiUtils.PROMPT_INPUT_PROTECTION_SHORT_DELAY_MS)
                                .build();

                modalDialogManager.showDialog(propertyModel, ModalDialogManager.ModalDialogType.TAB);
                recordDangerousDownloadDialogEvent(
                                DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_SHOW);
        }

        /**
         * Called to show a "Download blocked" dialog for dangerous download.
         * 
         * @param context            Context for showing the dialog.
         * @param modalDialogManager Manager for managing the modal dialog.
         * @param fileName           Name of the download file.
         * @param totalBytes         Total bytes of the file.
         * @param iconId             Icon ID of the blocked dialog.
         * @param callback           Callback to run when user dismisses the dialog
         *                           (always returns false).
         */
        public void showBlockedDialog(
                        Context context,
                        ModalDialogManager modalDialogManager,
                        String fileName,
                        long totalBytes,
                        int iconId,
                        Callback<Boolean> callback) {

                // Add debugging log
                System.out.println("DangerousDownloadDialog: showBlockedDialog called for file: " + fileName);

                var resources = context.getResources();

                // New professional message for organization policy
                String message = "Download has been blocked on this site by your organization by policy.";

                // Professional title
                String title = "Suspicious Website";
                String okButtonText = "OK";

                var controller = new ModalDialogProperties.Controller() {
                        @Override
                        public void onClick(PropertyModel model, int buttonType) {
                                // Add debugging log
                                System.out.println(
                                                "DangerousDownloadDialog: Button clicked, buttonType: " + buttonType);

                                // For blocked dialog, any button click should dismiss the dialog
                                // Check if it's the positive button (Close button)
                                if (buttonType == ModalDialogProperties.ButtonType.POSITIVE) {
                                        System.out.println(
                                                        "DangerousDownloadDialog: Close button confirmed, dismissing dialog");

                                        // First dismiss the dialog
                                        modalDialogManager.dismissDialog(
                                                        model, DialogDismissalCause.POSITIVE_BUTTON_CLICKED);

                                        // Then handle the callback
                                        if (callback != null) {
                                                callback.onResult(false); // Always return false for blocked downloads
                                        }

                                        recordDangerousDownloadDialogEvent(
                                                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CANCEL);
                                } else {
                                        System.out.println("DangerousDownloadDialog: Unexpected button type: "
                                                        + buttonType + ", but dismissing anyway");
                                        // Fallback: dismiss dialog for any button click
                                        if (callback != null) {
                                                callback.onResult(false);
                                        }
                                        modalDialogManager.dismissDialog(
                                                        model, DialogDismissalCause.POSITIVE_BUTTON_CLICKED);
                                        recordDangerousDownloadDialogEvent(
                                                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_CANCEL);
                                }
                        }

                        @Override
                        public void onDismiss(PropertyModel model, int dismissalCause) {
                                if (dismissalCause != DialogDismissalCause.POSITIVE_BUTTON_CLICKED) {
                                        if (callback != null)
                                                callback.onResult(false);
                                        recordDangerousDownloadDialogEvent(
                                                        DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_DISMISS);
                                }
                        }
                };

                // Add debugging for icon handling
                System.out.println("DangerousDownloadDialog: iconId = " + iconId);

                // Create a beautiful dialog with professional styling
                PropertyModel propertyModel = new PropertyModel.Builder(ModalDialogProperties.ALL_KEYS)
                                .with(ModalDialogProperties.CONTROLLER, controller)
                                .with(ModalDialogProperties.TITLE_ICON, null) // We'll handle custom styling
                                .with(ModalDialogProperties.BUTTON_STYLES,
                                                ModalDialogProperties.ButtonStyles.PRIMARY_FILLED_NO_NEGATIVE)
                                .with(ModalDialogProperties.BUTTON_TAP_PROTECTION_PERIOD_MS,
                                                UiUtils.PROMPT_INPUT_PROTECTION_SHORT_DELAY_MS)
                                .build();

                // Now add the custom view after the model is created
                propertyModel.set(ModalDialogProperties.CUSTOM_VIEW,
                                createCustomBlockedDialogView(context, fileName, totalBytes,
                                                controller, propertyModel));

                modalDialogManager.showDialog(propertyModel, ModalDialogManager.ModalDialogType.TAB);
                recordDangerousDownloadDialogEvent(
                                DangerousDownloadDialogEvent.DANGEROUS_DOWNLOAD_DIALOG_SHOW);
        }

        private View createCustomBlockedDialogView(Context context, String fileName, long totalBytes,
                        ModalDialogProperties.Controller controller, PropertyModel propertyModel) {
                // Create main container with rounded corners - increased size
                LinearLayout container = new LinearLayout(context);
                container.setOrientation(LinearLayout.VERTICAL);
                container.setPadding(24, 20, 24, 24); // Increased padding for more height

                // Set white background with rounded corners
                android.graphics.drawable.GradientDrawable background = new android.graphics.drawable.GradientDrawable();
                background.setColor(0xFFFFFFFF); // White background
                background.setCornerRadius(16 * context.getResources().getDisplayMetrics().density); // Modern corner
                                                                                                     // radius
                container.setBackground(background);

                // Create icon container
                LinearLayout iconContainer = new LinearLayout(context);
                iconContainer.setOrientation(LinearLayout.VERTICAL);
                iconContainer.setGravity(Gravity.CENTER);
                iconContainer.setPadding(0, 12, 0, 20); // Increased spacing for more height

                // Create green circular background for icon
                LinearLayout iconCircle = new LinearLayout(context);
                iconCircle.setOrientation(LinearLayout.VERTICAL);
                iconCircle.setGravity(Gravity.CENTER);

                // Set circular green background with border
                android.graphics.drawable.GradientDrawable circleBackground = new android.graphics.drawable.GradientDrawable();
                circleBackground.setShape(android.graphics.drawable.GradientDrawable.OVAL);
                circleBackground.setColor(0xFFE67E22); // Orange from your logo
                circleBackground.setStroke(2, 0xFFFDF2E9); 
                iconCircle.setBackground(circleBackground);

                // Set larger circle size
                int circleSize = (int) (52 * context.getResources().getDisplayMetrics().density); // Larger for better
                                                                                                  // proportion
                LinearLayout.LayoutParams circleParams = new LinearLayout.LayoutParams(circleSize, circleSize);
                circleParams.gravity = Gravity.CENTER;
                iconCircle.setLayoutParams(circleParams);

                // Create warning icon
                TextView warningIcon = new TextView(context);
                warningIcon.setText("⚠");
                warningIcon.setTextSize(26); // Larger size
                warningIcon.setTextColor(0xFF1A1A1A);
                warningIcon.setGravity(Gravity.CENTER);
                warningIcon.setTypeface(null, Typeface.BOLD);

                iconCircle.addView(warningIcon);
                iconContainer.addView(iconCircle);
                container.addView(iconContainer);

                // Create title
                TextView titleView = new TextView(context);
                titleView.setText("Download Blocked");
                titleView.setTextSize(19); // Larger size
                titleView.setTextColor(0xFF1A1A1A); // Dark text for contrast
                titleView.setTypeface(null, Typeface.BOLD);
                titleView.setGravity(Gravity.CENTER);
                titleView.setPadding(0, 0, 0, 16); // Increased spacing
                container.addView(titleView);

                // Create message
                TextView messageView = new TextView(context);
                messageView.setText("Download has been blocked on this site by your organization by policy.");
                messageView.setTextSize(15); // Larger size
                messageView.setTextColor(0xFF666666); // Medium gray
                messageView.setGravity(Gravity.CENTER);
                messageView.setLineSpacing(0, 1.3f); // Better line spacing
                messageView.setPadding(10, 0, 10, 24); // Increased padding
                container.addView(messageView);

                // Create OK button with green styling
                android.widget.Button closeButton = new android.widget.Button(context);
                closeButton.setText("OK");
                closeButton.setTextSize(16); // Larger size
                closeButton.setTextColor(0xFF000000);
                closeButton.setTypeface(null, Typeface.BOLD);

                // Create green button background with rounded corners
                android.graphics.drawable.GradientDrawable buttonBackground = new android.graphics.drawable.GradientDrawable();
                buttonBackground.setColor(0xFFD35400);
                buttonBackground.setCornerRadius(8 * context.getResources().getDisplayMetrics().density); // Rounded
                                                                                                          // corners
                closeButton.setBackground(buttonBackground);

                // Set button layout parameters with increased height
                LinearLayout.LayoutParams buttonParams = new LinearLayout.LayoutParams(
                                LinearLayout.LayoutParams.MATCH_PARENT,
                                (int) (48 * context.getResources().getDisplayMetrics().density) // Increased height
                );
                buttonParams.setMargins(0, 12, 0, 0); // Increased margin
                closeButton.setLayoutParams(buttonParams);

                // FIXED: Properly connect button to modal dialog system
                closeButton.setOnClickListener(v -> {
                        System.out.println("DangerousDownloadDialog: OK button clicked in custom view");
                        // Use the actual PropertyModel to properly dismiss the dialog
                        if (controller != null && propertyModel != null) {
                                controller.onClick(propertyModel, ModalDialogProperties.ButtonType.POSITIVE);
                        }
                });

                // Add button to container
                container.addView(closeButton);

                // Set container layout parameters with increased width and height
                LinearLayout.LayoutParams containerParams = new LinearLayout.LayoutParams(
                                (int) (340 * context.getResources().getDisplayMetrics().density), // Slightly increased
                                                                                                  // width - 340dp
                                LinearLayout.LayoutParams.WRAP_CONTENT // Let height adjust to content
                );
                containerParams.gravity = Gravity.CENTER;
                container.setLayoutParams(containerParams);

                return container;
        }

        /**
         * Collects dangerous download dialog UI event metrics.
         *
         * @param event The UI event to collect.
         */
        private static void recordDangerousDownloadDialogEvent(
                        @DangerousDownloadDialogEvent int event) {
                RecordHistogram.recordEnumeratedHistogram(
                                "Download.DangerousDialog.Events", event, DangerousDownloadDialogEvent.COUNT);
        }
}
