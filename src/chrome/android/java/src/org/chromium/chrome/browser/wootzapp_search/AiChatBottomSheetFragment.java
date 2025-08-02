// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.wootzapp_search;

import android.content.Context;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.bottomsheet.BottomSheetDialogFragment;

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

    private final List<ChatMessage> mMessages = new ArrayList<>();
    private ChatAdapter mAdapter;

    public static AiChatBottomSheetFragment newInstance(String searchQuery) {
        AiChatBottomSheetFragment fragment = new AiChatBottomSheetFragment();
        Bundle args = new Bundle();
        args.putString(ARG_SEARCH_QUERY, searchQuery);
        fragment.setArguments(args);
        return fragment;
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
        mGeminiApiClient = new GeminiApiClient("");
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
        ChatMessage loadingMessage = new ChatMessage("🤔 Thinking...", false);
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
                            
                            // Add the actual response
                            appendMessage(new ChatMessage(response, false));
                            
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
                            appendMessage(new ChatMessage(errorMessage, false));
                            
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
                    appendMessage(new ChatMessage("❌ Error: AI service not available", false));
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
}