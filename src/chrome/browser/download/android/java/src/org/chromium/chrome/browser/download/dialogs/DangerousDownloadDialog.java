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
import android.view.ViewGroup;
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
         * Called to show a "Download blocked" snackbar for dangerous download.
         * 
         * @param context            Context for showing the snackbar.
         * @param modalDialogManager Manager for managing the modal dialog (not used for snackbar).
         * @param fileName           Name of the download file.
         * @param totalBytes         Total bytes of the file.
         * @param iconId             Icon ID of the blocked dialog (not used for snackbar).
         * @param callback           Callback to run when user dismisses the snackbar
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
                System.out.println("DangerousDownloadDialog: showBlockedSnackbar called for file: " + fileName);

                // Create and show the snackbar
                showDownloadBlockedSnackbar(context, callback);
        }

        /**
         * Shows a custom snackbar for download blocking at the top of the screen.
         */
        private void showDownloadBlockedSnackbar(Context context, Callback<Boolean> callback) {
                // Find the root view (CoordinatorLayout or similar)
                View rootView = findRootView(context);
                if (rootView == null) {
                        // Fallback to callback if no root view found
                        if (callback != null) {
                                callback.onResult(false);
                        }
                        return;
                }

                // Create custom snackbar view matching download blocking styling
                View customSnackbarView = createCustomDownloadBlockedSnackbarView(context, callback);
                
                // Add the custom view to the root view
                if (rootView instanceof ViewGroup) {
                        ViewGroup rootGroup = (ViewGroup) rootView;
                        rootGroup.addView(customSnackbarView);
                        
                        // Auto-remove after duration
                        new android.os.Handler(android.os.Looper.getMainLooper()).postDelayed(() -> {
                                rootGroup.removeView(customSnackbarView);
                                if (callback != null) {
                                        callback.onResult(false);
                                }
                        }, 4000); // 4 seconds
                }
        }

        /**
         * Finds the root view for the Snackbar.
         */
        private View findRootView(Context context) {
                // Try to find the main activity's root view
                if (context instanceof android.app.Activity) {
                        android.app.Activity activity = (android.app.Activity) context;
                        View rootView = activity.findViewById(android.R.id.content);
                        if (rootView != null) {
                                // Look for CoordinatorLayout or suitable parent
                                ViewGroup parent = (ViewGroup) rootView;
                                for (int i = 0; i < parent.getChildCount(); i++) {
                                        View child = parent.getChildAt(i);
                                        if (child instanceof androidx.coordinatorlayout.widget.CoordinatorLayout) {
                                                return child;
                                        }
                                }
                                return rootView;
                        }
                }
                return null;
        }

        /**
         * Creates a custom snackbar view for download blocking positioned at the top.
         */
        private View createCustomDownloadBlockedSnackbarView(Context context, Callback<Boolean> callback) {
                // Create main snackbar container
                LinearLayout snackbarContainer = new LinearLayout(context);
                snackbarContainer.setOrientation(LinearLayout.VERTICAL);
                
                // Set white background with rounded corners
                android.graphics.drawable.GradientDrawable background = new android.graphics.drawable.GradientDrawable();
                background.setColor(0xFFFFFFFF); // White background
                background.setCornerRadius(8 * context.getResources().getDisplayMetrics().density);
                snackbarContainer.setBackground(background);
                
                // Add elevation for shadow effect
                snackbarContainer.setElevation(6 * context.getResources().getDisplayMetrics().density);
                
                // Create content container
                LinearLayout contentContainer = new LinearLayout(context);
                contentContainer.setOrientation(LinearLayout.HORIZONTAL);
                contentContainer.setPadding(24, 20, 24, 20);
                contentContainer.setGravity(Gravity.CENTER_VERTICAL);
                
                // Create warning icon with orange circle background
                LinearLayout iconContainer = new LinearLayout(context);
                iconContainer.setOrientation(LinearLayout.VERTICAL);
                iconContainer.setGravity(Gravity.CENTER);
                
                // Create orange circular background for icon
                android.graphics.drawable.GradientDrawable circleBackground = new android.graphics.drawable.GradientDrawable();
                circleBackground.setShape(android.graphics.drawable.GradientDrawable.OVAL);
                circleBackground.setColor(0xFFE67E22); // Orange color
                iconContainer.setBackground(circleBackground);
                
                // Set circle size
                int circleSize = (int) (36 * context.getResources().getDisplayMetrics().density);
                LinearLayout.LayoutParams circleParams = new LinearLayout.LayoutParams(circleSize, circleSize);
                circleParams.setMargins(0, 0, 16, 0);
                iconContainer.setLayoutParams(circleParams);
                
                // Create warning icon text
                TextView warningIcon = new TextView(context);
                warningIcon.setText("⚠");
                warningIcon.setTextSize(18);
                warningIcon.setTextColor(0xFFFFFFFF); // White text on orange background
                warningIcon.setGravity(Gravity.CENTER);
                warningIcon.setTypeface(null, Typeface.BOLD);
                
                iconContainer.addView(warningIcon);
                contentContainer.addView(iconContainer);
                
                // Create message container
                LinearLayout messageContainer = new LinearLayout(context);
                messageContainer.setOrientation(LinearLayout.VERTICAL);
                
                // Create title text
                TextView titleText = new TextView(context);
                titleText.setText("Download Blocked");
                titleText.setTextSize(18);
                titleText.setTextColor(0xFF1A1A1A);
                titleText.setTypeface(null, Typeface.BOLD);
                titleText.setPadding(0, 0, 0, 6);
                
                // Create message text
                TextView messageText = new TextView(context);
                messageText.setText("Download has been blocked on this site by your organization by policy.");
                messageText.setTextSize(15);
                messageText.setTextColor(0xFF666666);
                messageText.setLineSpacing(0, 1.3f);
                
                messageContainer.addView(titleText);
                messageContainer.addView(messageText);
                
                // Set layout weight to take up remaining space
                LinearLayout.LayoutParams messageParams = new LinearLayout.LayoutParams(
                        0, LinearLayout.LayoutParams.WRAP_CONTENT, 1.0f);
                messageContainer.setLayoutParams(messageParams);
                contentContainer.addView(messageContainer);
                
                // Add content to main container
                snackbarContainer.addView(contentContainer);
                
                // Create orange progress strip
                View progressStrip = new View(context);
                
                // Create orange background for progress strip
                android.graphics.drawable.GradientDrawable progressBackground = new android.graphics.drawable.GradientDrawable();
                progressBackground.setColor(0xFFE67E22); // Orange color
                progressBackground.setCornerRadii(new float[]{0, 0, 0, 0, 8, 8, 8, 8}); // Rounded bottom corners only
                progressStrip.setBackground(progressBackground);
                
                // Set initial strip dimensions (full width, 3dp height)
                LinearLayout.LayoutParams stripParams = new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.MATCH_PARENT,
                        (int) (3 * context.getResources().getDisplayMetrics().density)
                );
                progressStrip.setLayoutParams(stripParams);
                
                // Add progress strip to container
                snackbarContainer.addView(progressStrip);
                
                // Set snackbar container dimensions and position at TOP
                LinearLayout.LayoutParams containerParams = new LinearLayout.LayoutParams(
                        (int) (400 * context.getResources().getDisplayMetrics().density), // Fixed width
                        LinearLayout.LayoutParams.WRAP_CONTENT
                );
                containerParams.gravity = Gravity.TOP | Gravity.CENTER_HORIZONTAL; // Position at TOP
                containerParams.setMargins(16, 40, 16, 0); // Top margin for status bar
                snackbarContainer.setLayoutParams(containerParams);
                
                // Animate the progress strip shrinking from right to left over 4 seconds
                android.animation.ValueAnimator progressAnimator = android.animation.ValueAnimator.ofFloat(1.0f, 0.0f);
                progressAnimator.setDuration(4000); // 4 seconds
                progressAnimator.setInterpolator(new android.view.animation.LinearInterpolator());
                
                progressAnimator.addUpdateListener(animation -> {
                    float progress = (Float) animation.getAnimatedValue();
                    
                    // Update the width of the progress strip
                    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) progressStrip.getLayoutParams();
                    int containerWidth = snackbarContainer.getWidth();
                    if (containerWidth > 0) {
                        params.width = (int) (containerWidth * progress);
                        progressStrip.setLayoutParams(params);
                    }
                });
                
                // Start the animation after a short delay to ensure the view is laid out
                snackbarContainer.post(() -> {
                    progressAnimator.start();
                });
                
                return snackbarContainer;
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