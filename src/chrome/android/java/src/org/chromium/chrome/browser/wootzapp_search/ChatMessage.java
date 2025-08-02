// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.wootzapp_search;

/** Represents a chat message in the AI conversation. */
public class ChatMessage {
    private final String mMessage;
    private final boolean mIsUser;
    private final MessageType mType;

    public enum MessageType {
        USER,
        AI_RESPONSE,
        AI_ERROR,
        LOADING
    }

    public ChatMessage(String message, boolean isUser) {
        this.mMessage = message;
        this.mIsUser = isUser;
        this.mType = isUser ? MessageType.USER : MessageType.AI_RESPONSE;
    }

    public ChatMessage(String message, MessageType type) {
        this.mMessage = message;
        this.mIsUser = type == MessageType.USER;
        this.mType = type;
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

    public boolean isError() {
        return mType == MessageType.AI_ERROR;
    }

    public boolean isLoading() {
        return mType == MessageType.LOADING;
    }
} 