// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import android.text.Html;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Button;
import android.widget.LinearLayout;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;

import java.util.List;

/** Adapter for displaying chat messages. */
public class ChatAdapter extends RecyclerView.Adapter<ChatAdapter.ChatViewHolder> {
    private static final int VIEW_TYPE_USER = 1;
    private static final int VIEW_TYPE_AI = 2;
    private static final int VIEW_TYPE_ERROR = 3;
    private static final int VIEW_TYPE_LOADING = 4;

    private final List<ChatMessage> mMessages;
    private ConfigButtonClickListener mConfigButtonClickListener;

    public interface ConfigButtonClickListener {
        void onConfigButtonClick(String buttonText);
    }

    public ChatAdapter(List<ChatMessage> messages) {
        mMessages = messages;
    }

    public void setConfigButtonClickListener(ConfigButtonClickListener listener) {
        mConfigButtonClickListener = listener;
    }

    @NonNull
    @Override
    public ChatViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        int layoutId;
        switch (viewType) {
            case VIEW_TYPE_USER:
                layoutId = R.layout.chat_message_user;
                break;
            case VIEW_TYPE_ERROR:
                layoutId = R.layout.chat_message_ai;
                break;
            case VIEW_TYPE_LOADING:
                layoutId = R.layout.chat_message_ai;
                break;
            default:
                layoutId = R.layout.chat_message_ai;
                break;
        }
        View view = LayoutInflater.from(parent.getContext()).inflate(layoutId, parent, false);
        return new ChatViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ChatViewHolder holder, int position) {
        ChatMessage message = mMessages.get(position);
        
        // Always show the message text
        holder.messageText.setVisibility(View.VISIBLE);
        
        // Handle error messages with potential config buttons
        if (message.isError() && message.getButtonText() != null && !message.getButtonText().isEmpty()) {
            // This is an error message that needs a config button
            Spanned formattedText = formatMessageText(message.getMessage());
            holder.messageText.setText(formattedText);
            
            // Show and setup the config button
            if (holder.configButton == null) {
                // Create button dynamically
                AiModelClient aiClient = new AiModelClient();
                holder.configButton = aiClient.createConfigurationButton(holder.itemView.getContext(), message.getButtonText());
                
                // Make button smaller to fit inside message
                android.widget.LinearLayout.LayoutParams params = new android.widget.LinearLayout.LayoutParams(
                    android.widget.LinearLayout.LayoutParams.WRAP_CONTENT,
                    android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                );
                params.setMargins(0, 16, 0, 0); // Add top margin
                holder.configButton.setLayoutParams(params);
                holder.configButton.setPadding(24, 12, 24, 12); // Smaller padding
                holder.configButton.setTextSize(12f); // Smaller text
                
                // Add click listener
                holder.configButton.setOnClickListener(v -> {
                    if (mConfigButtonClickListener != null) {
                        mConfigButtonClickListener.onConfigButtonClick(message.getButtonText());
                    }
                });
                
                // Add to layout
                if (holder.itemView instanceof LinearLayout) {
                    ((LinearLayout) holder.itemView).addView(holder.configButton);
                }
            }
            holder.configButton.setVisibility(View.VISIBLE);
            holder.configButton.setText(message.getButtonText());
        } else {
            // Regular message or error without button
            // Hide button if exists
            if (holder.configButton != null) {
                holder.configButton.setVisibility(View.GONE);
            }
        }
        
        // Set the message text for all types
        Spanned formattedText = formatMessageText(message.getMessage());
        holder.messageText.setText(formattedText);
        
        if (message.isUser()) {
            holder.messageText.setTextColor(android.graphics.Color.BLACK);
            holder.messageText.setTextSize(android.util.TypedValue.COMPLEX_UNIT_SP, 16);
            
            int padding = (int) (16 * holder.itemView.getContext().getResources().getDisplayMetrics().density);
            holder.messageText.setPadding(padding, padding, padding, padding);
            
        } else {
            holder.messageText.setTextColor(android.graphics.Color.BLACK);
            holder.messageText.setTextSize(android.util.TypedValue.COMPLEX_UNIT_SP, 15);
            
            int padding = (int) (16 * holder.itemView.getContext().getResources().getDisplayMetrics().density);
            holder.messageText.setPadding(padding, padding, padding, padding);
            
            holder.messageText.setMovementMethod(LinkMovementMethod.getInstance());
        }
        
        holder.messageText.setLineSpacing(4, 1.2f);
    }

    private Spanned formatMessageText(String text) {
        if (text == null) return Html.fromHtml("");
        
        
        // Handle headers (## and ###)
        text = text.replaceAll("###\\s*(.*?)(?=\\n|$)", "<b><big>$1</big></b>");
        text = text.replaceAll("##\\s*(.*?)(?=\\n|$)", "<b><big><big>$1</big></big></b>");
        text = text.replaceAll("#\\s*(.*?)(?=\\n|$)", "<b><big><big><big>$1</big></big></big></b>");
        
        // Handle code blocks (```code```) - make them stand out
        text = text.replaceAll("```([\\s\\S]*?)```", "<tt><small><font color='#2E3440'>$1</font></small></tt>");
        
        // Handle inline code (`code`)
        text = text.replaceAll("`([^`]*?)`", "<tt><small><font color='#2E3440'>$1</font></small></tt>");
        
        // Handle numbered lists (1. 2. 3.)
        text = text.replaceAll("(\\d+)\\. ", "<b>$1.</b> ");
        
        // Handle bullet points (- or *)
        text = text.replaceAll("^[-*] ", "• ");
        text = text.replaceAll("\\n[-*] ", "<br>• ");
        
        // Handle bold text (**text**)
        text = text.replaceAll("\\*\\*(.*?)\\*\\*", "<b>$1</b>");
        
        // Handle italic text (*text*)
        text = text.replaceAll("\\*(.*?)\\*", "<i>$1</i>");
        
        // Handle step indicators
        text = text.replaceAll("(Step\\s*\\d+:?)", "<b>$1</b>");
        text = text.replaceAll("(First,|Second,|Third,|Next,|Then,|Finally,|Lastly,)", "<b>$1</b>");
        
        // Handle technical terms
        text = text.replaceAll("(Machine Learning|Data Science|AI|Artificial Intelligence|Deep Learning)", "<b>$1</b>");
        text = text.replaceAll("(Python|TensorFlow|PyTorch|Scikit-learn|Pandas|NumPy)", "<b>$1</b>");
        
        // Handle line breaks
        text = text.replaceAll("\n", "<br>");
        
        return Html.fromHtml(text, Html.FROM_HTML_MODE_COMPACT);
    }

    @Override
    public int getItemCount() {
        return mMessages.size();
    }

    @Override
    public int getItemViewType(int position) {
        ChatMessage message = mMessages.get(position);
        if (message.isUser()) {
            return VIEW_TYPE_USER;
        } else if (message.isError()) {
            return VIEW_TYPE_ERROR;
        } else if (message.isLoading()) {
            return VIEW_TYPE_LOADING;
        } else {
            return VIEW_TYPE_AI;
        }
    }

    static class ChatViewHolder extends RecyclerView.ViewHolder {
        TextView messageText;
        Button configButton;

        ChatViewHolder(View itemView) {
            super(itemView);
            messageText = itemView.findViewById(R.id.message_text);
            
            if (messageText != null) {
                int maxWidth = (int) (itemView.getContext().getResources().getDisplayMetrics().widthPixels * 0.85);
                messageText.setMaxWidth(maxWidth);

                int minWidth = (int) (itemView.getContext().getResources().getDisplayMetrics().widthPixels * 0.25);
                messageText.setMinWidth(minWidth);
                
                int minHeight = (int) (48 * itemView.getContext().getResources().getDisplayMetrics().density);
                messageText.setMinHeight(minHeight);
            }
        }
    }
}