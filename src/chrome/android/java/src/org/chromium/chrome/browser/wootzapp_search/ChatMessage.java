// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;


/** Represents a chat message in the AI conversation. */
public class ChatMessage {
    private final String mMessage;
    private final boolean mIsUser;
    private final MessageType mType;
    private final ContentType mContentType;

    public enum MessageType {
        USER,
        AI_RESPONSE,
        AI_ERROR,
        LOADING
    }

    public enum ContentType {
        TEXT,
        CODE,
        LIST,
        SECTION_HEADER,
        EVALUATION,
        DEPLOYMENT,
        STEP_BY_STEP,
        TECHNICAL_EXPLANATION
    }

    public ChatMessage(String message, boolean isUser) {
        this.mMessage = message;
        this.mIsUser = isUser;
        this.mType = isUser ? MessageType.USER : MessageType.AI_RESPONSE;
        this.mContentType = determineContentType(message);
    }

    public ChatMessage(String message, MessageType type) {
        this.mMessage = message;
        this.mIsUser = type == MessageType.USER;
        this.mType = type;
        this.mContentType = determineContentType(message);
    }

    public ChatMessage(String message, MessageType type, ContentType contentType) {
        this.mMessage = message;
        this.mIsUser = type == MessageType.USER;
        this.mType = type;
        this.mContentType = contentType;
    }

    private ContentType determineContentType(String message) {
        if (message == null) return ContentType.TEXT;
        
        String lowerMessage = message.toLowerCase();
        
        if (lowerMessage.contains("```") || lowerMessage.contains("`")) {
            return ContentType.CODE;
        }
        
        if (lowerMessage.contains("##") || lowerMessage.contains("###")) {
            return ContentType.SECTION_HEADER;
        }
        
        if (lowerMessage.contains("evaluation") || lowerMessage.contains("testing")) {
            return ContentType.EVALUATION;
        }
        
        if (lowerMessage.contains("deployment") || lowerMessage.contains("production")) {
            return ContentType.DEPLOYMENT;
        }
        
        if (lowerMessage.contains("step") || lowerMessage.contains("1.") || lowerMessage.contains("2.")) {
            return ContentType.STEP_BY_STEP;
        }
        
        if (lowerMessage.contains("data science") || lowerMessage.contains("machine learning") || 
            lowerMessage.contains("algorithm") || lowerMessage.contains("model")) {
            return ContentType.TECHNICAL_EXPLANATION;
        }
        
        if (lowerMessage.contains("- ") || lowerMessage.contains("* ") || lowerMessage.contains("1. ")) {
            return ContentType.LIST;
        }
        
        return ContentType.TEXT;
    }

    public String getMessage() {
        return mMessage;
    }

    public boolean isUser() {
        return mIsUser;
    }

    public MessageType getType() {
        return mType;
    }

    public ContentType getContentType() {
        return mContentType;
    }

    public boolean isError() {
        return mType == MessageType.AI_ERROR;
    }

    public boolean isLoading() {
        return mType == MessageType.LOADING;
    }
} 