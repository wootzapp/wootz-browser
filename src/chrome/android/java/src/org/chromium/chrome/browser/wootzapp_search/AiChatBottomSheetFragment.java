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
import android.widget.Button;
import android.widget.LinearLayout;
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
    private AiModelClient mAiModelClient;
    private String mInitialSearchQuery;
    private BottomSheetBehavior<View> mBehavior;
    private LinearLayout mChatContainer;
    private Button mConfigButton;

    private final List<ChatMessage> mMessages = new ArrayList<>();
    private ChatAdapter mAdapter;

    public static AiChatBottomSheetFragment newInstance(String searchQuery) {
        Log.e(TAG, "newInstance called with searchQuery: " + searchQuery);
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
        
        dialog.setCanceledOnTouchOutside(true);
        
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

        // Use the root LinearLayout as chat container
        mChatContainer = (LinearLayout) root;
        mRecyclerView = root.findViewById(R.id.chat_recycler_view);
        mInput = root.findViewById(R.id.chat_input);
        mSendBtn = root.findViewById(R.id.chat_send);

        mRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
        
        int spacingInPixels = (int) (4 * getResources().getDisplayMetrics().density);
        mRecyclerView.addItemDecoration(new RecyclerView.ItemDecoration() {
            @Override
            public void getItemOffsets(android.graphics.Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
                outRect.bottom = spacingInPixels;
                outRect.left = spacingInPixels;
                outRect.right = spacingInPixels;
                if (parent.getChildAdapterPosition(view) == 0) {
                    outRect.top = spacingInPixels;
                }
            }
        });
        
        mRecyclerView.setHasFixedSize(false);
        mRecyclerView.setItemAnimator(new androidx.recyclerview.widget.DefaultItemAnimator());

        mAiModelClient = new AiModelClient();

        if (getArguments() != null) {
            mInitialSearchQuery = getArguments().getString(ARG_SEARCH_QUERY);
        }

        mMessages.add(new ChatMessage("Hi, how can I help you?", false));

        mAdapter = new ChatAdapter(mMessages);
        mAdapter.setConfigButtonClickListener(buttonText -> {
            // Handle configuration button click
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
                
                // Close the AI chat bottom sheet after redirecting
                dismiss();
                
            } catch (org.chromium.chrome.browser.app.ChromeActivity.ChromeActivityNotFoundException e) {
                Log.e(TAG, "ChromeActivity not found: " + e);
                // Still close the bottom sheet even if there's an error
                dismiss();
            }
        });
        mRecyclerView.setAdapter(mAdapter);

        mSendBtn.setOnClickListener(v -> {
            handleSendClicked();
        });
        
        Log.d(TAG, "AiChatBottomSheetFragment setup completed");
        return root;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.e(TAG, "onActivityCreated called");

        View parent = (View) getView().getParent();
        if (parent != null) {
            parent.setBackgroundColor(android.graphics.Color.WHITE);

            mBehavior = BottomSheetBehavior.from(parent);
            
            addSlideInAnimation(parent);
            
            int screenHeight = getResources().getDisplayMetrics().heightPixels;
            int desiredHeight = (int) (screenHeight * 0.6);
            mBehavior.setPeekHeight(desiredHeight);
            mBehavior.setMaxHeight(desiredHeight);

            mBehavior.setDraggable(false);
            mBehavior.setHideable(false);
            
            mBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);

            mBehavior.addBottomSheetCallback(new BottomSheetBehavior.BottomSheetCallback() {
                @Override
                public void onStateChanged(@NonNull View bottomSheet, int newState) {
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
                
                int margin = (int) (4 * getResources().getDisplayMetrics().density);
                layoutParams.setMargins(margin, 0, margin, 0);
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
        
        String userText = mInput.getText().toString().trim();
        
        if (TextUtils.isEmpty(userText)) {
            Log.d(TAG, "User text is empty, returning");
            return;
        }

        // Hide any existing config buttons when user sends a new message
        hideAllConfigButtons();

        addSendButtonAnimation();
        closeKeyboard();
        appendMessage(new ChatMessage(userText, true));
        mInput.setText("");
        sendMessageToAI(userText);
    }

    private void sendMessageToAI(String message) {
        if (mSendBtn != null) {
            mSendBtn.setEnabled(false);
            mSendBtn.setImageResource(android.R.drawable.ic_menu_upload);
        }

        ChatMessage loadingMessage = new ChatMessage("🤔 Thinking...", ChatMessage.MessageType.LOADING);
        appendMessage(loadingMessage);

        if (mAiModelClient != null) {
            mAiModelClient.sendMessage(message, new AiModelClient.Callback() {
                @Override
                public void onSuccess(String response) {
                    if (getActivity() != null && !getActivity().isFinishing()) {
                        getActivity().runOnUiThread(() -> {
                            removeLastMessage();
                            
                            ChatMessage aiMessage = new ChatMessage(response, ChatMessage.MessageType.AI_RESPONSE);
                            appendMessage(aiMessage);
                            
                            // Hide any existing config buttons when we get a successful response
                            hideAllConfigButtons();
                            
                            if (mSendBtn != null) {
                                mSendBtn.setEnabled(true);
                                mSendBtn.setImageResource(android.R.drawable.ic_menu_send);
                            }
                        });
                    }
                }

                @Override
                public void onError(String error) {
                    if (getActivity() != null && !getActivity().isFinishing()) {
                        getActivity().runOnUiThread(() -> {
                            removeLastMessage();
                            
                            // Add error message to chat with embedded button for configuration errors
                            String errorMessage = "❌ Error: " + error;
                            ChatMessage errorMsg = new ChatMessage(errorMessage, ChatMessage.MessageType.AI_ERROR);
                            
                            // Mark if this error should show a config button
                            if (error.contains("API Key Not Configured") || error.contains("AI Model Not Configured")) {
                                String buttonText = error.contains("API Key") ? "🔑 Configure API Key" : "🤖 Configure AI Model";
                                errorMsg.setButtonText(buttonText);
                            }
                            
                            appendMessage(errorMsg);

                            if (mSendBtn != null) {
                                mSendBtn.setEnabled(true);
                                mSendBtn.setImageResource(android.R.drawable.ic_menu_send);
                            }
                        });
                    }
                }
            });
        } else {
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

    private void addSlideInAnimation(View view) {
        view.setTranslationY(view.getHeight());
        
        android.animation.ObjectAnimator slideIn = android.animation.ObjectAnimator.ofFloat(
            view, "translationY", view.getHeight(), 0f);
        slideIn.setDuration(500);
        slideIn.setInterpolator(new android.view.animation.DecelerateInterpolator());
        
        slideIn.start();
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
}