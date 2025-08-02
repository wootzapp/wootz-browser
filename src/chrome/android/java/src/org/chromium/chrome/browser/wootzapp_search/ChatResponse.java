// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.wootzapp_search;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/** Extracts the first text part from a Gemini response. */
public class ChatResponse {
    private final String mText;
    
    ChatResponse(String text) { 
        mText = text; 
    }
    
    public String getText() { 
        return mText; 
    }

    static ChatResponse fromJson(String json) throws JSONException {
        JSONObject o = new JSONObject(json);
        JSONArray cand = o.optJSONArray("candidates");
        if (cand != null && cand.length() > 0) {
            JSONObject content = cand.getJSONObject(0).optJSONObject("content");
            if (content != null) {
                JSONArray parts = content.optJSONArray("parts");
                if (parts != null && parts.length() > 0) {
                    String t = parts.getJSONObject(0).optString("text", "");
                    return new ChatResponse(t);
                }
            }
        }
        throw new JSONException("No text in response");
    }
} 