// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.wootzapp_search;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;

/**
 * Formats conversation history for the Gemini REST "generateContent" endpoint.
 *
 * Schema ➜  { "contents":[ { "role":"user|model", "parts":[{"text":"..."}] } ] }
 */
public class ChatRequest {

    private final List<ChatMessage> mMessages;

    public ChatRequest(List<ChatMessage> messages) {
        this.mMessages = messages;
    }

    public JSONObject toJson() throws JSONException {
        JSONArray contents = new JSONArray();

        for (ChatMessage m : mMessages) {
            JSONObject content = new JSONObject()
                    .put("role", m.isUser() ? "user" : "model")
                    .put("parts", new JSONArray()
                            .put(new JSONObject().put("text", m.getMessage())));

            contents.put(content);
        }
        return new JSONObject().put("contents", contents);
    }
} 