// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import android.content.Context;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;
import com.google.android.material.bottomsheet.BottomSheetDialog;
import com.google.android.material.bottomsheet.BottomSheetBehavior;

import org.chromium.chrome.R;

import java.util.ArrayList;
import java.util.List;

/** Bottom sheet fragment for AI chat functionality. */
public class AiChatBottomSheetFragment extends BottomSheetDialogFragment {
    private static final String TAG = "AiChatBottomSheetFragment";
    private static final String ARG_SEARCH_QUERY = "search_query";

    private RecyclerView mRecyclerView;
    private EditText mInput;
    private ImageButton mSendBtn;
    private GeminiApiClient mGeminiApiClient;
    private String mInitialSearchQuery;
    private BottomSheetBehavior<View> mBehavior;

    private final List<ChatMessage> mMessages = new ArrayList<>();
    private ChatAdapter mAdapter;

    public static AiChatBottomSheetFragment newInstance(String searchQuery) {
        AiChatBottomSheetFragment fragment = new AiChatBottomSheetFragment();
        Bundle args = new Bundle();
        args.putString(ARG_SEARCH_QUERY, searchQuery);
        fragment.setArguments(args);
        return fragment;
    }

    @NonNull
    @Override
    public android.app.Dialog onCreateDialog(Bundle savedInstanceState) {
        BottomSheetDialog dialog = (BottomSheetDialog) super.onCreateDialog(savedInstanceState);
        
        // Set the dialog to be canceled on touch outside
        dialog.setCanceledOnTouchOutside(true);
        
        // Remove the dark scrim behind the menu like AppMenu does
        Window window = dialog.getWindow();
        if (window != null) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        }
        
        return dialog;
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView called");
        
        View root = inflater.inflate(R.layout.fragment_ai_chat, container, false);
        Log.d(TAG, "Fragment layout inflated: " + root);

        mRecyclerView = root.findViewById(R.id.chat_recycler_view);
        mInput = root.findViewById(R.id.chat_input);
        mSendBtn = root.findViewById(R.id.chat_send);
        
        Log.d(TAG, "Views found - RecyclerView: " + mRecyclerView + 
              ", Input: " + mInput + ", SendBtn: " + mSendBtn);

        mRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));

        // Initialize Gemini API client
        mGeminiApiClient = new GeminiApiClient("AIzaSyCoNFODrVovsQEFa4nseHbv0d56eMqhtDU");
        Log.d(TAG, "GeminiApiClient initialized: " + mGeminiApiClient);

        // Get initial search query from arguments
        if (getArguments() != null) {
            mInitialSearchQuery = getArguments().getString(ARG_SEARCH_QUERY);
            Log.d(TAG, "Initial search query: " + mInitialSearchQuery);
        }

        // Add welcome message
        mMessages.add(new ChatMessage("Hi, how can I help you?", false));

        mAdapter = new ChatAdapter(mMessages);
        mRecyclerView.setAdapter(mAdapter);
        Log.d(TAG, "ChatAdapter set on RecyclerView");

        mSendBtn.setOnClickListener(v -> {
            Log.d(TAG, "Send button clicked");
            handleSendClicked();
        });
        
        Log.d(TAG, "AiChatBottomSheetFragment setup completed");
        return root;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.e(TAG, "onActivityCreated called");
        
        // Get the parent view (the bottom sheet container)
        View parent = (View) getView().getParent();
        if (parent != null) {
            // Set background to white
            parent.setBackgroundColor(android.graphics.Color.WHITE);

            // Get the behavior and configure it
            mBehavior = BottomSheetBehavior.from(parent);
            
            // Add slide-in animation
            addSlideInAnimation(parent);
            
            // Increase height to 700px for more content space
            int fixedHeight = (int) (500 * getResources().getDisplayMetrics().density);
            mBehavior.setPeekHeight(fixedHeight);
            mBehavior.setMaxHeight(fixedHeight);

            mBehavior.setDraggable(false);
            mBehavior.setHideable(false);
            
            // Set initial state to collapsed
            mBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);

            // Add callback to handle state changes
            mBehavior.addBottomSheetCallback(new BottomSheetBehavior.BottomSheetCallback() {
                @Override
                public void onStateChanged(@NonNull View bottomSheet, int newState) {
                    Log.d(TAG, "Bottom sheet state changed to: " + newState);
                    // Force the height to stay fixed
                    if (newState == BottomSheetBehavior.STATE_HIDDEN) {
                        mBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                    }
                }

                @Override
                public void onSlide(@NonNull View bottomSheet, float slideOffset) {
                    // Handle slide events if needed
                    Log.d(TAG, "Bottom sheet slide offset: " + slideOffset);
                }
            });

            if (parent.getLayoutParams() instanceof androidx.coordinatorlayout.widget.CoordinatorLayout.LayoutParams) {
                androidx.coordinatorlayout.widget.CoordinatorLayout.LayoutParams layoutParams = 
                    (androidx.coordinatorlayout.widget.CoordinatorLayout.LayoutParams) parent.getLayoutParams();
                
                // Reduce margins to increase width and reduce gaps
                int margin = (int) (16 * getResources().getDisplayMetrics().density);
                layoutParams.setMargins(margin, -margin, margin, margin);
                parent.setLayoutParams(layoutParams);
            }
            
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                float borderRadius = dpToPx(20);
                parent.setClipToOutline(true);
                parent.setOutlineProvider(new android.view.ViewOutlineProvider() {
                    @Override
                    public void getOutline(android.view.View view, android.graphics.Outline outline) {
                        outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), borderRadius);
                    }
                });
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume called");
        
        // Auto-search with initial query when AI tab is opened
        if (mInitialSearchQuery != null && !mInitialSearchQuery.isEmpty()) {
            Log.d(TAG, "Auto-searching with initial query: " + mInitialSearchQuery);
            // Add the initial query as a user message
            appendMessage(new ChatMessage(mInitialSearchQuery, true));
            // Send the message to AI
            sendMessageToAI(mInitialSearchQuery);
            // Clear the initial query to avoid duplicate processing
            mInitialSearchQuery = null;
        }
    }

    private void handleSendClicked() {
        Log.d(TAG, "handleSendClicked called");
        
        String userText = mInput.getText().toString().trim();
        Log.d(TAG, "User text: '" + userText + "'");
        
        if (TextUtils.isEmpty(userText)) {
            Log.d(TAG, "User text is empty, returning");
            return;
        }

        // Add button animation
        addSendButtonAnimation();

        // Close keyboard
        closeKeyboard();

        // Add user message to UI
        appendMessage(new ChatMessage(userText, true));
        mInput.setText("");
        
        // Send message to AI
        sendMessageToAI(userText);
    }

    private void sendMessageToAI(String message) {
        // Disable send button and show loading
        if (mSendBtn != null) {
            mSendBtn.setEnabled(false);
            mSendBtn.setImageResource(android.R.drawable.ic_menu_upload);
            Log.d(TAG, "Send button disabled and loading icon shown");
        }

        // Add a loading message to show AI is thinking
        ChatMessage loadingMessage = new ChatMessage("🤔 Thinking...", ChatMessage.MessageType.LOADING);
        appendMessage(loadingMessage);

        // Call Gemini API
        Log.d(TAG, "Calling Gemini API with text: '" + message + "'");
        if (mGeminiApiClient != null) {
            mGeminiApiClient.sendMessage(message, new GeminiApiClient.Callback() {
                @Override
                public void onSuccess(String response) {
                    Log.d(TAG, "Gemini API success - Response: " + response);
                    if (getActivity() != null && !getActivity().isFinishing()) {
                        getActivity().runOnUiThread(() -> {
                            // Remove the loading message
                            removeLastMessage();
                            
                            // Add the actual response with proper content type detection
                            ChatMessage aiMessage = new ChatMessage(response, ChatMessage.MessageType.AI_RESPONSE);
                            appendMessage(aiMessage);
                            
                            // Re-enable send button
                            if (mSendBtn != null) {
                                mSendBtn.setEnabled(true);
                                mSendBtn.setImageResource(android.R.drawable.ic_menu_send);
                                Log.d(TAG, "Send button re-enabled with success response");
                            }
                        });
                    }
                }

                @Override
                public void onError(String error) {
                    Log.e(TAG, "Gemini API error: " + error);
                    if (getActivity() != null && !getActivity().isFinishing()) {
                        getActivity().runOnUiThread(() -> {
                            // Remove the loading message
                            removeLastMessage();
                            
                            // Add error message to chat
                            String errorMessage = "❌ Error: " + error;
                            ChatMessage errorMsg = new ChatMessage(errorMessage, ChatMessage.MessageType.AI_ERROR);
                            appendMessage(errorMsg);
                            
                            // Show toast for additional feedback
                            Toast.makeText(getContext(), "AI Error: " + error, Toast.LENGTH_LONG).show();
                            
                            // Re-enable send button
                            if (mSendBtn != null) {
                                mSendBtn.setEnabled(true);
                                mSendBtn.setImageResource(android.R.drawable.ic_menu_send);
                                Log.d(TAG, "Send button re-enabled after error");
                            }
                        });
                    }
                }
            });
        } else {
            Log.e(TAG, "GeminiApiClient is null!");
            // Handle null client case
            if (getActivity() != null && !getActivity().isFinishing()) {
                getActivity().runOnUiThread(() -> {
                    removeLastMessage();
                    ChatMessage errorMsg = new ChatMessage("❌ Error: AI service not available", ChatMessage.MessageType.AI_ERROR);
                    appendMessage(errorMsg);
                    if (mSendBtn != null) {
                        mSendBtn.setEnabled(true);
                        mSendBtn.setImageResource(android.R.drawable.ic_menu_send);
                    }
                });
            }
        }
    }

    private void appendMessage(ChatMessage msg) {
        Log.d(TAG, "Appending message: " + msg.getMessage() + ", isUser: " + msg.isUser());
        mMessages.add(msg);
        mAdapter.notifyItemInserted(mMessages.size() - 1);
        mRecyclerView.scrollToPosition(mMessages.size() - 1);
        
        // Add animation to the new message
        mRecyclerView.post(() -> {
            View lastChild = mRecyclerView.getLayoutManager().getChildAt(mMessages.size() - 1);
            if (lastChild != null) {
                addMessageAnimation(lastChild);
            }
        });
        
        Log.d(TAG, "Message appended, total messages: " + mMessages.size());
    }

    private void removeLastMessage() {
        if (!mMessages.isEmpty()) {
            Log.d(TAG, "Removing last message");
            mMessages.remove(mMessages.size() - 1);
            mAdapter.notifyItemRemoved(mMessages.size());
        }
    }

    private void closeKeyboard() {
        if (mInput != null) {
            InputMethodManager imm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null) {
                imm.hideSoftInputFromWindow(mInput.getWindowToken(), 0);
                Log.d(TAG, "Keyboard closed");
            }
        }
    }

    private int dpToPx(int dp) {
        float density = getResources().getDisplayMetrics().density;
        return Math.round((float) dp * density);
    }

    // Add slide-in animation
    private void addSlideInAnimation(View view) {
        // Set initial position (off-screen)
        view.setTranslationY(view.getHeight());
        
        // Create slide-in animation
        android.animation.ObjectAnimator slideIn = android.animation.ObjectAnimator.ofFloat(
            view, "translationY", view.getHeight(), 0f);
        slideIn.setDuration(500);
        slideIn.setInterpolator(new android.view.animation.DecelerateInterpolator());
        
        // Start animation
        slideIn.start();
    }

    // Add fade-in animation for messages
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

    // Add pulse animation for send button
    private void addSendButtonAnimation() {
        if (mSendBtn != null) {
            android.animation.ObjectAnimator scaleX = android.animation.ObjectAnimator.ofFloat(
                mSendBtn, "scaleX", 1f, 1.2f, 1f);
            android.animation.ObjectAnimator scaleY = android.animation.ObjectAnimator.ofFloat(
                mSendBtn, "scaleY", 1f, 1.2f, 1f);
            
            android.animation.AnimatorSet animatorSet = new android.animation.AnimatorSet();
            animatorSet.playTogether(scaleX, scaleY);
            animatorSet.setDuration(200);
            animatorSet.setInterpolator(new android.view.animation.OvershootInterpolator());
            
            animatorSet.start();
        }
    }

    // Add typing indicator animation
    private void addTypingAnimation() {
        View typingIndicator = getView().findViewById(R.id.typing_indicator);
        if (typingIndicator != null) {
            android.animation.ObjectAnimator alpha = android.animation.ObjectAnimator.ofFloat(
                typingIndicator, "alpha", 0f, 1f, 0f);
            alpha.setDuration(1500);
            alpha.setRepeatCount(android.animation.ValueAnimator.INFINITE);
            alpha.setRepeatMode(android.animation.ValueAnimator.RESTART);
            alpha.start();
        }
    }

    // Add background gradient animation
    private void addBackgroundAnimation(View view) {
        android.graphics.drawable.GradientDrawable gradient = new android.graphics.drawable.GradientDrawable(
            android.graphics.drawable.GradientDrawable.Orientation.TL_BR,
            new int[]{
                0xFF4A90E2,  // Blue
                0xFF7B68EE,  // Medium Slate Blue
                0xFF9370DB   // Medium Purple
            }
        );
        gradient.setCornerRadius(dpToPx(16));
        gradient.setGradientType(android.graphics.drawable.GradientDrawable.LINEAR_GRADIENT);
        
        view.setBackground(gradient);
        
        // Animate gradient colors
        android.animation.ValueAnimator animator = android.animation.ValueAnimator.ofFloat(0f, 1f);
        animator.setDuration(4000);
        animator.setRepeatCount(android.animation.ValueAnimator.INFINITE);
        animator.setRepeatMode(android.animation.ValueAnimator.REVERSE);
        
        animator.addUpdateListener(new android.animation.ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(android.animation.ValueAnimator animation) {
                float value = (Float) animation.getAnimatedValue();
                
                // Create smooth color transitions
                int color1 = android.graphics.Color.argb(255, 
                    (int)(74 + (255 - 74) * value),   // R: 74 -> 255
                    (int)(144 + (107 - 144) * value), // G: 144 -> 107
                    (int)(226 + (107 - 226) * value)); // B: 226 -> 107
                
                int color2 = android.graphics.Color.argb(255, 
                    (int)(123 + (78 - 123) * value),  // R: 123 -> 78
                    (int)(104 + (205 - 104) * value), // G: 104 -> 205
                    (int)(238 + (196 - 238) * value)); // B: 238 -> 196
                
                int color3 = android.graphics.Color.argb(255, 
                    (int)(147 + (69 - 147) * value),  // R: 147 -> 69
                    (int)(112 + (183 - 112) * value), // G: 112 -> 183
                    (int)(219 + (209 - 219) * value)); // B: 219 -> 209
                
                gradient.setColors(new int[]{color1, color2, color3});
            }
        });
        
        animator.start();
    }

    private void addDynamicGradientAnimation(View view) {
        android.graphics.drawable.GradientDrawable gradient = new android.graphics.drawable.GradientDrawable(
            android.graphics.drawable.GradientDrawable.Orientation.TL_BR,
            new int[]{
                0xFF4A90E2,  // Primary Blue
                0xFF7B68EE,  // Purple
                0xFF9370DB,  // Light Purple
                0xFF4ECDC4   // Teal
            }
        );
        gradient.setCornerRadius(dpToPx(16));
        gradient.setGradientType(android.graphics.drawable.GradientDrawable.LINEAR_GRADIENT);
        
        view.setBackground(gradient);
        
        // Create multiple color transitions
        android.animation.ValueAnimator animator = android.animation.ValueAnimator.ofFloat(0f, 1f);
        animator.setDuration(6000);
        animator.setRepeatCount(android.animation.ValueAnimator.INFINITE);
        animator.setRepeatMode(android.animation.ValueAnimator.REVERSE);
        
        animator.addUpdateListener(new android.animation.ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(android.animation.ValueAnimator animation) {
                float value = (Float) animation.getAnimatedValue();
                
                // Create a rainbow-like effect
                int color1 = android.graphics.Color.HSVToColor(new float[]{
                    (240 + value * 60) % 360,  // Hue: Blue to Purple
                    0.8f,                       // Saturation
                    0.9f                        // Value
                });
                
                int color2 = android.graphics.Color.HSVToColor(new float[]{
                    (280 + value * 60) % 360,  // Hue: Purple to Pink
                    0.7f,                       // Saturation
                    0.8f                        // Value
                });
                
                int color3 = android.graphics.Color.HSVToColor(new float[]{
                    (320 + value * 60) % 360,  // Hue: Pink to Red
                    0.6f,                       // Saturation
                    0.7f                        // Value
                });
                
                int color4 = android.graphics.Color.HSVToColor(new float[]{
                    (0 + value * 60) % 360,    // Hue: Red to Orange
                    0.5f,                       // Saturation
                    0.6f                        // Value
                });
                
                gradient.setColors(new int[]{color1, color2, color3, color4});
            }
        });
        
        animator.start();
    }
}