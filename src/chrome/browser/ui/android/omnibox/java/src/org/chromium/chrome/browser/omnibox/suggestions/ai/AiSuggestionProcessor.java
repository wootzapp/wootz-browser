package org.chromium.chrome.browser.omnibox.suggestions.ai;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import org.chromium.base.Log;
import org.chromium.chrome.browser.omnibox.R;
import org.chromium.chrome.browser.omnibox.styles.OmniboxDrawableState;
import org.chromium.chrome.browser.omnibox.styles.OmniboxImageSupplier;
import org.chromium.chrome.browser.omnibox.styles.SuggestionSpannable;
import org.chromium.chrome.browser.omnibox.suggestions.SuggestionHost;
import org.chromium.chrome.browser.omnibox.suggestions.base.BaseSuggestionViewProcessor;
import org.chromium.chrome.browser.omnibox.suggestions.base.BaseSuggestionViewProperties;
import org.chromium.components.omnibox.AutocompleteMatch;
import org.chromium.components.omnibox.suggestions.OmniboxSuggestionUiType;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.url.GURL;

import java.util.Optional;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

/** A class that handles model and view creation for AI suggestions. */
public class AiSuggestionProcessor extends BaseSuggestionViewProcessor {
    private static final String TAG = "AiSuggestionProcessor";
    private static final int AI_SUGGESTION_COUNT = 6;
    private static final long DEBOUNCE_DELAY_MS = 2000; // 2 seconds
    
    private String mCurrentQuery = "";
    private String mLastProcessedQuery = "";
    private Handler mHandler = new Handler(Looper.getMainLooper());
    private Runnable mDebounceRunnable;
    private boolean mIsLoading = false;
    private boolean mIsGenerating = false;
    private Runnable mRefreshCallback;
    private AiSuggestionDescriptionClient mDescriptionClient;
    private Map<Integer, String> mAiDescriptions = new HashMap<>();
    private List<AutocompleteMatch> mCurrentSuggestions = new ArrayList<>();

    public AiSuggestionProcessor(
            @NonNull Context context,
            @NonNull SuggestionHost suggestionHost,
            @NonNull Optional<OmniboxImageSupplier> imageSupplier) {
        super(context, suggestionHost, imageSupplier);
        mDescriptionClient = new AiSuggestionDescriptionClient();
    }

    /**
     * Set a callback to be called when the debounce completes and suggestions should be refreshed.
     */
    public void setRefreshCallback(Runnable refreshCallback) {
        mRefreshCallback = refreshCallback;
    }

    /**
     * Store current suggestions for AI description generation.
     */
    public void setCurrentSuggestions(List<AutocompleteMatch> suggestions) {
        mCurrentSuggestions.clear();
        if (suggestions != null) {
            // Only take the first 6 suggestions
            int count = Math.min(suggestions.size(), AI_SUGGESTION_COUNT);
            for (int i = 0; i < count; i++) {
                mCurrentSuggestions.add(suggestions.get(i));
            }
        }
    }

    @Override
    public boolean doesProcessSuggestion(@NonNull AutocompleteMatch suggestion, int position) {
        // NEW FLOW: Generate AI suggestions instead of Google suggestions
        // Only show AI suggestions when user is typing (has a query) and for the first 6 positions
        return !mCurrentQuery.isEmpty() && position < AI_SUGGESTION_COUNT;

    }

    @Override
    public int getViewTypeId() {
        return OmniboxSuggestionUiType.AI_SUGGESTION;
    }

    @Override
    public @NonNull PropertyModel createModel() {
        return new PropertyModel(AiSuggestionViewProperties.ALL_KEYS);
    }

    @Override
    public void populateModel(
            @NonNull AutocompleteMatch suggestion, @NonNull PropertyModel model, int position) {
        super.populateModel(suggestion, model, position);
        setStateForAiSuggestion(model, position, suggestion);

        // // Set the icon for AI suggestions using the existing icon system
        // OmniboxDrawableState icon = OmniboxDrawableState.forSmallIcon(mContext, R.drawable.ic_wootzapp, false);
        // model.set(BaseSuggestionViewProperties.ICON, icon);
        
        // Override the click handler for AI suggestions
        model.set(BaseSuggestionViewProperties.ON_CLICK, 
                 () -> onAiSuggestionClicked(suggestion, position));
    }

    /**
     * Update the current query to determine when to show AI suggestions.
     * NEW FLOW: Generate AI suggestions on every text change with proper cleanup.
     */
    public void updateQuery(String query) {
        String newQuery = query != null ? query : "";
        
        // Cancel any existing debounce runnable
        if (mDebounceRunnable != null) {
            mHandler.removeCallbacks(mDebounceRunnable);
        }
        
        mCurrentQuery = newQuery;
        
        // NEW FLOW: Generate AI suggestions on every text change
        if (!newQuery.isEmpty()) {
            // Clear old suggestions immediately to prevent inconsistency
            mAiDescriptions.clear();
            mLastProcessedQuery = "";
            
            // Set loading state immediately
            mIsLoading = true;
            
            // Create new debounce runnable
            mDebounceRunnable = () -> {
                // Only generate if query has changed and we're not already generating
                if (!newQuery.equals(mLastProcessedQuery) && !mIsGenerating) {
                    mLastProcessedQuery = newQuery;
                    generateAiDescriptions();
                }
            };
            
            // Schedule the debounce (300ms for faster response)
            mHandler.postDelayed(mDebounceRunnable, 300);
            
            // Trigger a refresh to show loading state
            if (mRefreshCallback != null) {
                mRefreshCallback.run();
            }
        } else {
            mIsLoading = false;
            mLastProcessedQuery = "";
            mAiDescriptions.clear();
            // Trigger refresh to clear suggestions
            if (mRefreshCallback != null) {
                mRefreshCallback.run();
            }
        }
        
        // OLD FLOW (commented out): Generate AI suggestions with debounce to prevent multiple API calls
        /*
        // NEW FLOW: Generate AI suggestions with debounce to prevent multiple API calls
        if (!newQuery.isEmpty()) {
            // Set loading state immediately
            mIsLoading = true;
            
            // Create new debounce runnable
            mDebounceRunnable = () -> {
                // Only generate if query has changed and we're not already generating
                if (!newQuery.equals(mLastProcessedQuery) && !mIsGenerating) {
                    mLastProcessedQuery = newQuery;
                    generateAiDescriptions();
                }
            };
            
            // Schedule the debounce (500ms to prevent too many API calls)
            mHandler.postDelayed(mDebounceRunnable, 500);
            
            // Trigger a refresh to show loading state
            if (mRefreshCallback != null) {
                mRefreshCallback.run();
            }
        } else {
            mIsLoading = false;
            mLastProcessedQuery = "";
            mAiDescriptions.clear();
        }
        */
    }

    /**
     * Override to handle new suggestions received.
     * NEW FLOW: Don't generate AI suggestions here to prevent duplicate calls.
     */
    @Override
    public void onSuggestionsReceived() {
        // NEW FLOW: Don't generate AI suggestions here to prevent duplicate calls
        // AI suggestions are generated in updateQuery() with proper debouncing
        Log.d(TAG, "Suggestions received, but AI suggestions are handled in updateQuery()");
        
        // OLD FLOW (commented out): Generate AI suggestions based on current query
        /*
        // Generate AI suggestions immediately when we have a query
        if (!mCurrentQuery.isEmpty()) {
            Log.i("Kartik", "Generating AI suggestions for query: " + mCurrentQuery);
            generateAiDescriptions();
        }
        */
        
        // OLD FLOW (commented out): Generate AI descriptions for Google suggestions on every suggestion update
        /*
        // Generate AI descriptions for Google suggestions immediately when received
        if (!mCurrentQuery.isEmpty() && !mCurrentSuggestions.isEmpty()) {
            Log.i("Kartik", "New suggestions received, generating AI descriptions for: " + mCurrentQuery);
            generateAiDescriptions();
        }
        */
    }

    /**
     * Generate AI suggestions (both headers and descriptions) based on current query.
     */
    private void generateAiDescriptions() {
        if (mCurrentQuery.isEmpty()) {
            Log.d(TAG, "No query to generate AI suggestions for");
            return;
        }

        // Prevent multiple concurrent API calls
        if (mIsGenerating) {
            Log.d(TAG, "Already generating AI suggestions, skipping");
            return;
        }

        mIsGenerating = true;
        Log.d(TAG, "Generating AI suggestions for query: " + mCurrentQuery);
        
        // NEW FLOW: Generate AI suggestions based on the current query instead of Google suggestions
        mDescriptionClient.generateAiSuggestions(mCurrentQuery, AI_SUGGESTION_COUNT, new AiSuggestionDescriptionClient.Callback() {
            @Override
            public void onSuccess(List<AiSuggestionDescriptionClient.SuggestionDescription> descriptions) {
                Log.d(TAG, "Received " + descriptions.size() + " AI suggestions");
                
                // Store AI suggestions by position
                mAiDescriptions.clear();
                for (AiSuggestionDescriptionClient.SuggestionDescription desc : descriptions) {
                    mAiDescriptions.put(desc.position, desc.description);
                }
                
                // Stop loading and generating states
                mIsLoading = false;
                mIsGenerating = false;
                
                // Trigger UI refresh to show the AI suggestions
                if (mRefreshCallback != null) {
                    mRefreshCallback.run();
                }
            }

            @Override
            public void onError(String error) {
                Log.w(TAG, "Failed to generate AI suggestions: " + error);
                // Clear any existing suggestions on error
                mAiDescriptions.clear();
                // Stop loading and generating states even on error
                mIsLoading = false;
                mIsGenerating = false;
                
                // Trigger UI refresh to hide shimmer
                if (mRefreshCallback != null) {
                    mRefreshCallback.run();
                }
            }
        });
        
        // OLD FLOW (commented out): Generate AI descriptions for Google suggestions
        /*
        if (mCurrentSuggestions.isEmpty()) {
            Log.d(TAG, "No suggestions to generate descriptions for");
            return;
        }

        Log.d(TAG, "Generating AI descriptions for " + mCurrentSuggestions.size() + " suggestions");
        
        mDescriptionClient.generateDescriptions(mCurrentSuggestions, new AiSuggestionDescriptionClient.Callback() {
            @Override
            public void onSuccess(List<AiSuggestionDescriptionClient.SuggestionDescription> descriptions) {
                Log.d(TAG, "Received " + descriptions.size() + " AI descriptions");
                
                // Store descriptions by position
                mAiDescriptions.clear();
                for (AiSuggestionDescriptionClient.SuggestionDescription desc : descriptions) {
                    mAiDescriptions.put(desc.position, desc.description);
                }
                
                // Stop loading state now that we have AI descriptions
                mIsLoading = false;
                
                // Trigger UI refresh to show the descriptions
                if (mRefreshCallback != null) {
                    mRefreshCallback.run();
                }
            }

            @Override
            public void onError(String error) {
                Log.w(TAG, "Failed to generate AI descriptions: " + error);
                // Clear any existing descriptions on error
                mAiDescriptions.clear();
                // Stop loading state even on error
                mIsLoading = false;
                
                // Trigger UI refresh to hide shimmer
                if (mRefreshCallback != null) {
                    mRefreshCallback.run();
                }
            }
        });
        */
    }

    private void setStateForAiSuggestion(PropertyModel model, int position, AutocompleteMatch suggestion) {
        // Set AI-specific properties
        model.set(AiSuggestionViewProperties.IS_AI_SUGGESTION, true);
        model.set(AiSuggestionViewProperties.IS_LOADING, mIsLoading);
        
        if (mIsLoading) {
            // NEW FLOW: Show shimmer effect for both header and subtext when loading AI suggestions
            // Clear any existing text content to show shimmer effect
            model.set(AiSuggestionViewProperties.TEXT_LINE_1_TEXT, null);
            model.set(AiSuggestionViewProperties.TEXT_LINE_2_TEXT, null);
            return;
        }
        
        // NEW FLOW: Set AI-generated header text
        String aiHeader = mAiDescriptions.get(position * 2); // Even positions for headers
        if (aiHeader != null && !aiHeader.isEmpty()) {
            // Clean the header text - remove any quotes or extra formatting
            String cleanHeader = aiHeader.trim();
            if (cleanHeader.startsWith("\"") && cleanHeader.endsWith("\"")) {
                cleanHeader = cleanHeader.substring(1, cleanHeader.length() - 1);
            }
            SuggestionSpannable headerSpannable = new SuggestionSpannable(cleanHeader);
            model.set(AiSuggestionViewProperties.TEXT_LINE_1_TEXT, headerSpannable);
        } else {
            // Clear header if no AI suggestion available
            model.set(AiSuggestionViewProperties.TEXT_LINE_1_TEXT, null);
        }
        
        // NEW FLOW: Set AI-generated subtext (description)
        String aiDescription = mAiDescriptions.get(position * 2 + 1); // Odd positions for descriptions
        if (aiDescription != null && !aiDescription.isEmpty()) {
            // Clean the description text - remove any quotes or extra formatting
            String cleanDescription = aiDescription.trim();
            if (cleanDescription.startsWith("\"") && cleanDescription.endsWith("\"")) {
                cleanDescription = cleanDescription.substring(1, cleanDescription.length() - 1);
            }
            SuggestionSpannable subSpannable = new SuggestionSpannable(cleanDescription);
            model.set(AiSuggestionViewProperties.TEXT_LINE_2_TEXT, subSpannable);
        } else {
            // Clear description if no AI suggestion available
            model.set(AiSuggestionViewProperties.TEXT_LINE_2_TEXT, null);
        }
        
        // OLD FLOW (commented out): Use Google suggestions with AI descriptions
        /*
        // Always show the header text from the Google suggestion immediately
        String headerText = suggestion.getDisplayText();

        String description = suggestion.getDescription(); // Suggestion description
        String fillIntoEdit = suggestion.getFillIntoEdit(); // What goes in omnibox
        String destinationUrl = suggestion.getUrl().getSpec(); // Target URL

        Log.i("Kartik", "Description: " + description);
        Log.i("Kartik", "Fill into edit: " + fillIntoEdit);
        Log.i("Kartik", "Destination URL: " + destinationUrl);
        
        SuggestionSpannable headerSpannable = new SuggestionSpannable(headerText);
        model.set(AiSuggestionViewProperties.TEXT_LINE_1_TEXT, headerSpannable);
        
        if (mIsLoading) {
            // Show shimmer effect for subtext when loading AI descriptions
            // Don't set subtext content - the view will show shimmer effect
            return;
        }
        
        // Set AI-generated subtext only if we have a description
        String subText = mAiDescriptions.get(position);
        if (subText != null && !subText.isEmpty()) {
            SuggestionSpannable subSpannable = new SuggestionSpannable(subText);
            model.set(AiSuggestionViewProperties.TEXT_LINE_2_TEXT, subSpannable);
        }
        // No fallback text - keep showing shimmer until AI description arrives
        */
    }

    private void onAiSuggestionClicked(AutocompleteMatch suggestion, int position) {
        // NEW FLOW: Handle AI suggestions
        // Get the AI-generated header text for this position
        String aiHeader = mAiDescriptions.get(position * 2);
        if (aiHeader == null || aiHeader.isEmpty()) {
            Log.w("Kartik", "No AI header found for position: " + position);
            return;
        }
        
        // Clean the header text - remove any quotes or extra formatting
        String cleanHeader = aiHeader.trim();
        if (cleanHeader.startsWith("\"") && cleanHeader.endsWith("\"")) {
            cleanHeader = cleanHeader.substring(1, cleanHeader.length() - 1);
        }
        
        // Create a search URL with the cleaned AI-generated header text
        // URL encode the header to handle spaces and special characters properly
        String encodedHeader;
        try {
            encodedHeader = java.net.URLEncoder.encode(cleanHeader, "UTF-8");
        } catch (java.io.UnsupportedEncodingException e) {
            Log.w("Kartik", "Failed to encode header, using original: " + e.getMessage());
            encodedHeader = cleanHeader;
        }
        String searchUrl = "wootzapp://chat/?q=" + encodedHeader;
        
        Log.i("Kartik", "AI suggestion clicked - Original header: '" + aiHeader + "', Cleaned: '" + cleanHeader + "', Encoded: '" + encodedHeader + "', Final URL: " + searchUrl);
        
        // Create a GURL for the search URL
        GURL searchGUrl = new GURL(searchUrl);
        
        // Use the suggestion host to navigate to the search URL
        mSuggestionHost.onSuggestionClicked(suggestion, position, searchGUrl);
        
        // OLD FLOW (commented out): Handle Google suggestions with AI descriptions
        /*
        // Use the original Google suggestion URL for navigation
        GURL originalUrl = suggestion.getUrl();
        
        Log.i("Kartik", "AI-enhanced Google suggestion clicked, navigating to: " + originalUrl.getSpec());
        
        // Use the suggestion host to navigate to the original Google suggestion URL
        mSuggestionHost.onSuggestionClicked(suggestion, position, originalUrl);
        */
        
        // OLD FLOW (commented out): Custom wootzapp scheme handling
        /*
        // Get the header text from the suggestion
        String headerText = suggestion.getDisplayText();
        
        // Create the wootzapp://chat URL with the header text
        String chatUrl = "wootzapp://chat/?q=" + headerText;
        
        Log.i("Kartik", "AI Suggestion clicked, navigating to: " + chatUrl);
        
        // Create a GURL for the custom scheme
        GURL wootzappUrl = new GURL(chatUrl);
        
        // Use the suggestion host to navigate to the wootzapp URL
        mSuggestionHost.onSuggestionClicked(suggestion, position, wootzappUrl);
        */
    }
}
