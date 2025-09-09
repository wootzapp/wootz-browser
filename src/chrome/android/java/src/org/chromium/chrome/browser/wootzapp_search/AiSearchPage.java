 // Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.wootzapp_search;

import android.app.Activity;
import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.annotation.ColorInt;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.base.CallbackController;
import org.chromium.base.TraceEvent;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ui.native_page.BasicNativePage;
import org.chromium.chrome.browser.ui.native_page.NativePageHost;
import org.chromium.components.browser_ui.styles.ChromeColors;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.ui.text.EmptyTextWatcher;

import java.util.ArrayList;
import java.util.List;

/** Native page for AI chat functionality. */
public class AiSearchPage extends BasicNativePage {
    private static final String TAG = "AiSearchPage";

    private final Activity mActivity;
    private final Context mContext;
    private final NativePageHost mHost;
    private final String mTitle;
    private final int mBackgroundColor;
    private final String mUrl;
    private final String mHostName;

    private View mView;
    private EditText mChatInput;
    private ImageButton mSendButton;
    private RecyclerView mRecyclerView;
    private AiModelClient mAiModelClient;
    private CallbackController mCallbackController = new CallbackController();
    private List<ChatMessage> mMessages = new ArrayList<>();
    private ChatAdapter mAdapter;
    private String mInitialQuery;

    public AiSearchPage(
            Activity activity,
            NativePageHost host,
            String url) {
        super(host);
        
        TraceEvent.begin(TAG + ".constructor()");
        
        mActivity = activity;
        mContext = activity;
        mHost = host;
        mTitle = mContext.getString(R.string.chat_title);
        mBackgroundColor = ChromeColors.getSurfaceColor(mContext, R.dimen.default_elevation_4);
        mUrl = url;
        mHostName = UrlConstants.AI_CHAT_HOST;
        mInitialQuery = extractQueryFromUrl(url);
        
        mAiModelClient = new AiModelClient();
        
        initializeView();
        
        TraceEvent.end(TAG + ".constructor()");
    }

    private String extractQueryFromUrl(String url) {
        if (url == null || !url.contains("?")) return "";
        
        try {
            String query = url.substring(url.indexOf("?") + 1);
            Log.d(TAG, "Extracted query string: " + query);
            
            if (query.startsWith("q=")) {
                String rawQuery = query.substring(2);
                // URL decode the query to handle encoded characters like %20, %21, etc.
                String decodedQuery = java.net.URLDecoder.decode(rawQuery, "UTF-8");
                Log.d(TAG, "Extracted query from URL - Raw: '" + rawQuery + "', Decoded: '" + decodedQuery + "'");
                return decodedQuery;
            } else {
                Log.d(TAG, "Query parameter doesn't start with 'q=': " + query);
            }
        } catch (Exception e) {
            Log.e(TAG, "Error extracting query from URL: " + url, e);
        }
        return "";
    }

    private void initializeView() {
        LayoutInflater inflater = LayoutInflater.from(mContext);
        mView = inflater.inflate(R.layout.fragment_ai_chat, null);

        mChatInput = mView.findViewById(R.id.chat_input);
        mSendButton = mView.findViewById(R.id.chat_send);
        mRecyclerView = mView.findViewById(R.id.chat_recycler_view);

        // Set up RecyclerView
        mRecyclerView.setLayoutManager(new LinearLayoutManager(mContext));
        mAdapter = new ChatAdapter(mMessages);
        mRecyclerView.setAdapter(mAdapter);

        // Set up config button click listener
        mAdapter.setConfigButtonClickListener(buttonText -> {
            final String extensionId = "lhgoolpdddhhfahbnofaomjhfhfjfhop";
            
            // Hide all config buttons after click
            hideAllConfigButtons();
            
            try {
                org.chromium.chrome.browser.app.ChromeActivity activity = 
                    org.chromium.chrome.browser.app.ChromeActivity.getChromeActivity();
                
                // Check if the extension is installed
                boolean extensionExists = org.chromium.chrome.browser.extensions.Extensions.getExtensionsInfo().stream()
                        .anyMatch(ext -> ext.getId().equals(extensionId));
                
                if (extensionExists) {
                    // If extension is installed, open it directly
                    org.chromium.chrome.browser.extensions.OpenExtensionsById.openExtensionByIdNative(extensionId);
                } else {
                    // If extension is not installed, navigate to flow-store in a new tab
                    if (activity.getActivityTab() != null) {
                        // Create a new tab instead of using the current one
                        activity.getTabCreator(false).createNewTab(
                            new org.chromium.content_public.browser.LoadUrlParams("wootzapp://flow-store"),
                            org.chromium.chrome.browser.tab.TabLaunchType.FROM_CHROME_UI,
                            activity.getActivityTab());
                    } else {
                        // Fallback to intent if no active tab
                        android.content.Intent intent = new android.content.Intent(android.content.Intent.ACTION_VIEW);
                        intent.setData(android.net.Uri.parse("wootzapp://flow-store"));
                        activity.startActivity(intent);
                    }
                }
                
            } catch (org.chromium.chrome.browser.app.ChromeActivity.ChromeActivityNotFoundException e) {
                Log.e(TAG, "ChromeActivity not found: " + e);
            }
        });

        // Add initial greeting message
        mMessages.add(new ChatMessage("Hi, how can I help you?", false));
        mAdapter.notifyItemInserted(mMessages.size() - 1);

        // Set up send button click listener
        mSendButton.setOnClickListener(v -> {
            handleSendClicked();
        });

        // Set up text watcher to enable/disable send button
        mChatInput.addTextChangedListener(new EmptyTextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {
                mSendButton.setEnabled(!s.toString().trim().isEmpty());
            }
        });

        // Initially disable send button
        mSendButton.setEnabled(false);

        // Set initial query if provided
        if (mInitialQuery != null && !mInitialQuery.isEmpty()) {
            Log.d(TAG, "Auto-searching with initial query: " + mInitialQuery);
            // Add the initial query as a user message
            appendMessage(new ChatMessage(mInitialQuery, true));
            // Send the message to AI
            sendMessageToAI(mInitialQuery);
            // Clear the initial query to avoid duplicate processing
            mInitialQuery = null;
        }

        initWithView(mView);
    }

    private void handleSendClicked() {
        String userText = mChatInput.getText().toString().trim();
        
        if (userText.isEmpty()) {
            Log.d(TAG, "User text is empty, returning");
            return;
        }

        // Hide any existing config buttons when user sends a new message
        hideAllConfigButtons();

        addSendButtonAnimation();
        closeKeyboard();
        appendMessage(new ChatMessage(userText, true));
        mChatInput.setText("");
        sendMessageToAI(userText);
    }

    private void sendMessageToAI(String message) {
        if (mSendButton != null) {
            mSendButton.setEnabled(false);
            mSendButton.setImageResource(android.R.drawable.ic_menu_upload);
        }

        ChatMessage loadingMessage = new ChatMessage("�� Thinking...", ChatMessage.MessageType.LOADING);
        appendMessage(loadingMessage);

        if (mAiModelClient != null) {
            mAiModelClient.sendMessage(message, new AiModelClient.Callback() {
                @Override
                public void onSuccess(String response) {
                    if (mActivity != null && !mActivity.isFinishing()) {
                        mActivity.runOnUiThread(() -> {
                            removeLastMessage();
                            
                            ChatMessage aiMessage = new ChatMessage(response, ChatMessage.MessageType.AI_RESPONSE);
                            appendMessage(aiMessage);
                            
                            // Hide any existing config buttons when we get a successful response
                            hideAllConfigButtons();
                            
                            if (mSendButton != null) {
                                mSendButton.setEnabled(true);
                                mSendButton.setImageResource(android.R.drawable.ic_menu_send);
                            }
                        });
                    }
                }

                @Override
                public void onError(String error) {
                    if (mActivity != null && !mActivity.isFinishing()) {
                        mActivity.runOnUiThread(() -> {
                            removeLastMessage();
                            
                            // Add error message to chat with embedded button for configuration errors
                            String errorMessage = "❌ Error: " + error;
                            ChatMessage errorMsg = new ChatMessage(errorMessage, ChatMessage.MessageType.AI_ERROR);
                            
                            // Mark if this error should show a config button
                            if (error.contains("API Key Not Configured") || error.contains("AI Model Not Configured")) {
                                String buttonText = error.contains("API Key") ? "�� Configure API Key" : "�� Configure AI Model";
                                errorMsg.setButtonText(buttonText);
                            }
                            
                            appendMessage(errorMsg);

                            if (mSendButton != null) {
                                mSendButton.setEnabled(true);
                                mSendButton.setImageResource(android.R.drawable.ic_menu_send);
                            }
                        });
                    }
                }
            });
        } else {
            if (mActivity != null && !mActivity.isFinishing()) {
                mActivity.runOnUiThread(() -> {
                    removeLastMessage();
                    ChatMessage errorMsg = new ChatMessage("❌ Error: AI service not available", ChatMessage.MessageType.AI_ERROR);
                    appendMessage(errorMsg);
                    if (mSendButton != null) {
                        mSendButton.setEnabled(true);
                        mSendButton.setImageResource(android.R.drawable.ic_menu_send);
                    }
                });
            }
        }
    }

    private void appendMessage(ChatMessage msg) {
        mMessages.add(msg);
        mAdapter.notifyItemInserted(mMessages.size() - 1);
        mRecyclerView.scrollToPosition(mMessages.size() - 1);
        
        mRecyclerView.post(() -> {
            View lastChild = mRecyclerView.getLayoutManager().getChildAt(mMessages.size() - 1);
            if (lastChild != null) {
                addMessageAnimation(lastChild);
            }
        });
    }

    private void removeLastMessage() {
        if (!mMessages.isEmpty()) {
            Log.d(TAG, "Removing last message");
            mMessages.remove(mMessages.size() - 1);
            mAdapter.notifyItemRemoved(mMessages.size());
        }
    }

    private void closeKeyboard() {
        if (mChatInput != null) {
            android.view.inputmethod.InputMethodManager imm = (android.view.inputmethod.InputMethodManager) mContext.getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null) {
                imm.hideSoftInputFromWindow(mChatInput.getWindowToken(), 0);
                Log.d(TAG, "Keyboard closed");
            }
        }
    }

    private void addMessageAnimation(View messageView) {
        messageView.setAlpha(0f);
        messageView.setTranslationY(50f);
        
        android.animation.ObjectAnimator fadeIn = android.animation.ObjectAnimator.ofFloat(
            messageView, "alpha", 0f, 1f);
        android.animation.ObjectAnimator slideUp = android.animation.ObjectAnimator.ofFloat(
            messageView, "translationY", 50f, 0f);
        
        android.animation.AnimatorSet animatorSet = new android.animation.AnimatorSet();
        animatorSet.playTogether(fadeIn, slideUp);
        animatorSet.setDuration(300);
        animatorSet.setInterpolator(new android.view.animation.DecelerateInterpolator());
        
        animatorSet.start();
    }

    private void addSendButtonAnimation() {
        if (mSendButton != null) {
            android.animation.ObjectAnimator scaleX = android.animation.ObjectAnimator.ofFloat(
                mSendButton, "scaleX", 1f, 1.2f, 1f);
            android.animation.ObjectAnimator scaleY = android.animation.ObjectAnimator.ofFloat(
                mSendButton, "scaleY", 1f, 1.2f, 1f);
            
            android.animation.AnimatorSet animatorSet = new android.animation.AnimatorSet();
            animatorSet.playTogether(scaleX, scaleY);
            animatorSet.setDuration(200);
            animatorSet.setInterpolator(new android.view.animation.OvershootInterpolator());
            
            animatorSet.start();
        }
    }
    
    /**
     * Hides all configuration buttons by clearing button text from error messages
     */
    private void hideAllConfigButtons() {
        boolean changedAny = false;
        for (ChatMessage message : mMessages) {
            if (message.isError() && message.getButtonText() != null) {
                message.setButtonText(null);
                changedAny = true;
            }
        }
        if (changedAny) {
            mAdapter.notifyDataSetChanged();
            Log.d(TAG, "Cleared configuration buttons from error messages");
        }
    }

    // NativePage interface implementation
    @Override
    public String getTitle() {
        return mTitle;
    }

    @Override
    public String getHost() {
        return mHostName;
    }

    @Override
    public int getBackgroundColor() {
        return mBackgroundColor;
    }

    @Override
    public @ColorInt int getToolbarTextBoxBackgroundColor(@ColorInt int defaultColor) {
        return ChromeColors.getSurfaceColor(mContext, R.dimen.default_elevation_4);
    }

    @Override
    public @ColorInt int getToolbarSceneLayerBackground(@ColorInt int defaultColor) {
        return getBackgroundColor();
    }

    @Override
    public boolean needsToolbarShadow() {
        return true;
    }

    @Override
    public void updateForUrl(String url) {
        // Handle URL updates if needed
    }

    @Override
    public void destroy() {
        if (mCallbackController != null) {
            mCallbackController.destroy();
            mCallbackController = null;
        }
        super.destroy();
    }
}