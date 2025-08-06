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

    public ChatAdapter(List<ChatMessage> messages) {
        mMessages = messages;
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
        
        // Format the message text with enhanced styling
        Spanned formattedText = formatMessageText(message.getMessage());
        
        if (message.isUser()) {
            holder.messageText.setText(formattedText);
            holder.messageText.setTextColor(android.graphics.Color.BLACK);
        } else {
            holder.messageText.setText(formattedText);
            holder.messageText.setTextColor(android.graphics.Color.BLACK);
            
            // Enable clickable links for AI messages
            holder.messageText.setMovementMethod(LinkMovementMethod.getInstance());
        }
    }

    private Spanned formatMessageText(String text) {
        if (text == null) return Html.fromHtml("");
        
        // Handle different types of content sections
        
        // 1. Handle section headers (## Section Name)
        text = text.replaceAll("##\\s*(.*?)(?=\\n|$)", "<h3 style='color: #FF6B35; font-weight: bold; margin: 8px 0;'>$1</h3>");
        
        // 2. Handle subsection headers (### Subsection)
        text = text.replaceAll("###\\s*(.*?)(?=\\n|$)", "<h4 style='color: #FF8C42; font-weight: bold; margin: 6px 0;'>$1</h4>");
        
        // 3. Handle bullet points and lists
        text = text.replaceAll("^\\s*[-*]\\s+(.*?)(?=\\n|$)", "<li style='margin: 4px 0; padding-left: 16px;'>• $1</li>");
        text = text.replaceAll("^\\s*(\\d+)\\.\\s+(.*?)(?=\\n|$)", "<li style='margin: 4px 0; padding-left: 16px;'>$1. $2</li>");
        
        // 4. Handle bold text (**text**)
        text = text.replaceAll("\\*\\*(.*?)\\*\\*", "<b style='color: #FF6B35;'>$1</b>");
        
        // 5. Handle italic text (*text*)
        text = text.replaceAll("\\*(.*?)\\*", "<i style='color: #666;'>$1</i>");
        
        // 6. Handle code blocks (```code```)
        text = text.replaceAll("```(.*?)```", "<code style='background-color: #f5f5f5; padding: 8px; border-radius: 4px; font-family: monospace; display: block; margin: 8px 0;'>$1</code>");
        
        // 7. Handle inline code (`code`)
        text = text.replaceAll("`(.*?)`", "<code style='background-color: #f0f0f0; padding: 2px 4px; border-radius: 3px; font-family: monospace;'>$1</code>");
        
        // 8. Handle emphasis sections (__text__)
        text = text.replaceAll("__(.*?)__", "<span style='background-color: #FFF3E0; padding: 2px 4px; border-radius: 3px;'>$1</span>");
        
        // 9. Handle important sections (!!text!!)
        text = text.replaceAll("!!(.*?)!!", "<span style='background-color: #FFEBEE; color: #D32F2F; padding: 4px 8px; border-radius: 4px; font-weight: bold;'>$1</span>");
        
        // 10. Handle quotes (> text)
        text = text.replaceAll("^>\\s*(.*?)(?=\\n|$)", "<blockquote style='border-left: 4px solid #FF6B35; padding-left: 12px; margin: 8px 0; color: #666;'>$1</blockquote>");
        
        // 11. Handle line breaks
        text = text.replaceAll("\n", "<br>");
        
        // 12. Handle paragraphs (group consecutive text)
        text = text.replaceAll("(<br>){2,}", "</p><p>");
        text = "<p style='margin: 4px 0; line-height: 1.4;'>" + text + "</p>";
        
        // 13. Handle special formatting for different content types
        text = handleSpecialContentTypes(text);
        
        return Html.fromHtml(text, Html.FROM_HTML_MODE_COMPACT);
    }

    private String handleSpecialContentTypes(String text) {
        // Handle code examples
        if (text.contains("```java") || text.contains("```python") || text.contains("```javascript")) {
            text = text.replaceAll("```(java|python|javascript|html|css|xml)(.*?)```", 
                "<div style='background-color: #2d3748; color: #e2e8f0; padding: 12px; border-radius: 6px; margin: 8px 0; font-family: monospace;'><div style='color: #ffd700; margin-bottom: 4px;'>$1</div>$2</div>");
        }
        
        // Handle data science sections
        if (text.toLowerCase().contains("data science") || text.toLowerCase().contains("machine learning")) {
            text = text.replaceAll("(Data Science|Machine Learning|AI|Artificial Intelligence)", 
                "<span style='color: #4A90E2; font-weight: bold;'>$1</span>");
        }
        
        // Handle evaluation sections
        if (text.toLowerCase().contains("evaluation") || text.toLowerCase().contains("testing")) {
            text = text.replaceAll("(Evaluation|Testing|Performance|Accuracy)", 
                "<span style='color: #FF8C42; font-weight: bold;'>$1</span>");
        }
        
        // Handle deployment sections
        if (text.toLowerCase().contains("deployment") || text.toLowerCase().contains("production")) {
            text = text.replaceAll("(Deployment|Production|Integration)", 
                "<span style='color: #4CAF50; font-weight: bold;'>$1</span>");
        }
        
        // Handle step-by-step instructions
        if (text.contains("Step") || text.contains("step")) {
            text = text.replaceAll("(Step\\s*\\d+:|step\\s*\\d+:)", 
                "<span style='background-color: #E3F2FD; color: #1976D2; padding: 2px 6px; border-radius: 3px; font-weight: bold;'>$1</span>");
        }
        
        return text;
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

        ChatViewHolder(View itemView) {
            super(itemView);
            messageText = itemView.findViewById(R.id.message_text);
        }
    }
}