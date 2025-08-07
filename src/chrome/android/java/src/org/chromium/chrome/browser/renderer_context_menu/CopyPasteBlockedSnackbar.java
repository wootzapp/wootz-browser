package org.chromium.chrome.browser.renderer_context_menu;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.coordinatorlayout.widget.CoordinatorLayout;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.chromium.ui.base.WindowAndroid;

/**
 * Native Android Snackbar implementation for copy-paste blocking.
 * Matches the download blocking styling with warning icon, title, message, and
 * orange progress strip.
 */
@JNINamespace("chrome")
public class CopyPasteBlockedSnackbar {
    private static final int SNACKBAR_DURATION = 4000; // 4 seconds
    private static final String TAG = "CopyPasteBlockedSnackbar";

    /**
     * Shows a custom Snackbar with the copy-paste blocked message matching download
     * blocking styling.
     * 
     * @param windowAndroid The WindowAndroid instance
     * @param message       The message to display
     */
    @CalledByNative
    public static void show(@NonNull WindowAndroid windowAndroid, @NonNull String message) {
        if (windowAndroid.getContext().get() == null) {
            return;
        }

        Context context = windowAndroid.getContext().get();

        // Find the root view (CoordinatorLayout or similar)
        View rootView = findRootView(context);
        if (rootView == null) {
            // Fallback to Toast if no root view found
            showFallbackToast(context, message);
            return;
        }

        // Create custom snackbar view matching download blocking styling
        View customSnackbarView = createCustomSnackbarView(context, message);

        // Add the custom view to the root view
        if (rootView instanceof ViewGroup) {
            ViewGroup rootGroup = (ViewGroup) rootView;
            rootGroup.addView(customSnackbarView);

            // Auto-remove after duration
            new Handler(Looper.getMainLooper()).postDelayed(() -> {
                rootGroup.removeView(customSnackbarView);
            }, SNACKBAR_DURATION);
        }
    }

    /**
     * Creates a custom snackbar view matching the download blocking styling.
     */
    private static View createCustomSnackbarView(Context context, String message) {
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
        titleText.setText("Copy-Paste Blocked");
        titleText.setTextSize(18);
        titleText.setTextColor(0xFF1A1A1A);
        titleText.setTypeface(null, Typeface.BOLD);
        titleText.setPadding(0, 0, 0, 6);

        // Create message text
        TextView messageText = new TextView(context);
        messageText.setText(message);
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
        progressBackground.setCornerRadii(new float[] { 0, 0, 0, 0, 8, 8, 8, 8 }); // Rounded bottom corners only
        progressStrip.setBackground(progressBackground);

        // Set initial strip dimensions (full width, 3dp height)
        LinearLayout.LayoutParams stripParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                (int) (3 * context.getResources().getDisplayMetrics().density));
        progressStrip.setLayoutParams(stripParams);

        // Add progress strip to container
        snackbarContainer.addView(progressStrip);

        // Set snackbar container dimensions and position
        LinearLayout.LayoutParams containerParams = new LinearLayout.LayoutParams(
                (int) (400 * context.getResources().getDisplayMetrics().density), // Fixed width like download blocking
                LinearLayout.LayoutParams.WRAP_CONTENT);
        containerParams.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        containerParams.setMargins(16, 0, 16, 40);
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
     * Finds the root view for the Snackbar.
     */
    private static View findRootView(Context context) {
        // Try to find the main activity's root view
        if (context instanceof android.app.Activity) {
            android.app.Activity activity = (android.app.Activity) context;
            View rootView = activity.findViewById(android.R.id.content);
            if (rootView != null) {
                // Look for CoordinatorLayout or suitable parent
                ViewGroup parent = (ViewGroup) rootView;
                for (int i = 0; i < parent.getChildCount(); i++) {
                    View child = parent.getChildAt(i);
                    if (child instanceof CoordinatorLayout) {
                        return child;
                    }
                }
                return rootView;
            }
        }
        return null;
    }

    /**
     * Fallback to Toast if Snackbar cannot be shown.
     */
    private static void showFallbackToast(Context context, String message) {
        new Handler(Looper.getMainLooper()).post(() -> {
            Toast toast = Toast.makeText(context, message, Toast.LENGTH_LONG);
            toast.show();
        });
    }
}