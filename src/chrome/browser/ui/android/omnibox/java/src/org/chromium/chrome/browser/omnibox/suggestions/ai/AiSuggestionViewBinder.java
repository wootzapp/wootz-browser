package org.chromium.chrome.browser.omnibox.suggestions.ai;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.ColorInt;

import org.chromium.chrome.browser.omnibox.R;
import org.chromium.chrome.browser.omnibox.styles.OmniboxResourceProvider;
import org.chromium.chrome.browser.omnibox.styles.SuggestionSpannable;
import org.chromium.chrome.browser.omnibox.suggestions.SuggestionCommonProperties;
import org.chromium.chrome.browser.ui.theme.BrandedColorScheme;
import org.chromium.ui.modelutil.PropertyKey;
import org.chromium.ui.modelutil.PropertyModel;

/** Properties associated with the AI suggestion view. */
public class AiSuggestionViewBinder {
    /**
     * @see PropertyModelChangeProcessor.ViewBinder#bind(Object, Object, Object)
     */
    public static void bind(PropertyModel model, View view, PropertyKey propertyKey) {
        if (propertyKey == AiSuggestionViewProperties.TEXT_LINE_1_TEXT) {
            TextView tv = view.findViewById(R.id.line_1);
            SuggestionSpannable span = model.get(AiSuggestionViewProperties.TEXT_LINE_1_TEXT);
            tv.setText(span);
        } else if (propertyKey == AiSuggestionViewProperties.TEXT_LINE_2_TEXT) {
            updateSubtextContent(view, model);
        } else if (propertyKey == AiSuggestionViewProperties.IS_LOADING) {
            updateLoadingState(view, model);
        } else if (propertyKey == SuggestionCommonProperties.COLOR_SCHEME) {
            // For AI suggestions, we use custom colors defined in the layout XML
            // No need to override the color scheme here
        } else if (propertyKey == AiSuggestionViewProperties.IS_AI_SUGGESTION) {
            // Update the view to show it's an AI suggestion
            updateAiSuggestionAppearance(view, model);
        }
    }

    private static void updateSubtextContent(View view, PropertyModel model) {
        TextView subtextView = view.findViewById(R.id.line_2);
        View shimmerView = view.findViewById(R.id.subtext_shimmer);
        final SuggestionSpannable span = model.get(AiSuggestionViewProperties.TEXT_LINE_2_TEXT);
        
        if (!TextUtils.isEmpty(span)) {
            // Stop shimmer animation first
            stopShimmerAnimation(shimmerView);
            
            // Hide shimmer and show subtext
            shimmerView.setVisibility(View.GONE);
            subtextView.setVisibility(View.VISIBLE);
            
            // Start character-by-character typing animation
            startTypingAnimation(subtextView, span.toString());
        } else {
            subtextView.setVisibility(View.GONE);
            shimmerView.setVisibility(View.VISIBLE);
        }
    }

    private static void updateLoadingState(View view, PropertyModel model) {
        final boolean isLoading = model.get(AiSuggestionViewProperties.IS_LOADING);
        
        View shimmerView = view.findViewById(R.id.subtext_shimmer);
        TextView subtextView = view.findViewById(R.id.line_2);
        
        if (isLoading) {
            // Show shimmer effect for subtext
            shimmerView.setVisibility(View.VISIBLE);
            subtextView.setVisibility(View.GONE);
            
            // Start shimmer animation
            startShimmerAnimation(shimmerView);
        } else {
            // Stop shimmer animation when loading is complete
            stopShimmerAnimation(shimmerView);
            shimmerView.setVisibility(View.GONE);
            // Subtext will be shown when TEXT_LINE_2_TEXT is set
        }
    }

    private static void startShimmerAnimation(View shimmerView) {
        // Set the shimmer animation drawable
        shimmerView.setBackgroundResource(R.drawable.ai_suggestion_loading_shimmer);
        
        // Start the animation if it's an AnimationDrawable
        if (shimmerView.getBackground() instanceof AnimationDrawable) {
            ((AnimationDrawable) shimmerView.getBackground()).start();
        }
    }

    private static void stopShimmerAnimation(View shimmerView) {
        // Stop the animation if it's an AnimationDrawable
        if (shimmerView.getBackground() instanceof AnimationDrawable) {
            ((AnimationDrawable) shimmerView.getBackground()).stop();
        }
        // Clear the background to stop any animation
        shimmerView.setBackground(null);
    }

    private static void startTypingAnimation(TextView textView, String fullText) {
        // Clear any existing background
        textView.setBackground(null);
        
        // Start character-by-character typing animation
        Handler handler = new Handler(Looper.getMainLooper());
        textView.setText(""); // Start with empty text
        
        // Type each character with a delay for smooth animation
        for (int i = 0; i <= fullText.length(); i++) {
            final int charIndex = i;
            handler.postDelayed(() -> {
                if (charIndex <= fullText.length()) {
                    textView.setText(fullText.substring(0, charIndex));
                }
            }, i * 20); // 20ms delay between characters for faster display
        }
    }

    private static void updateAiSuggestionAppearance(View view, PropertyModel model) {
        final boolean isAiSuggestion = model.get(AiSuggestionViewProperties.IS_AI_SUGGESTION);
        if (isAiSuggestion) {
            // The background color is already set in the layout XML
            // No need to override it programmatically
        }
    }
}
