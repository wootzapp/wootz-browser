// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/** Extracts text from different AI provider responses. */
public class ChatResponse {
    private final String mText;
    
    ChatResponse(String text) { 
        mText = text; 
    }
    
    public String getText() { 
        return mText; 
    }

    static ChatResponse fromJson(String json, String provider) throws JSONException {
        switch (provider) {
            case "GEMINI":
                return parseGeminiResponse(json);
            case "OPENAI":
                return parseOpenAIResponse(json);
            case "ANTHROPIC":
                return parseAnthropicResponse(json);
            default:
                return parseGenericResponse(json);
        }
    }

    private static ChatResponse parseGeminiResponse(String json) throws JSONException {
        JSONObject response = new JSONObject(json);
        JSONArray candidates = response.getJSONArray("candidates");
        
        if (candidates.length() > 0) {
            JSONObject candidate = candidates.getJSONObject(0);
            JSONObject content = candidate.getJSONObject("content");
            JSONArray parts = content.getJSONArray("parts");
            
            if (parts.length() > 0) {
                JSONObject part = parts.getJSONObject(0);
                String text = part.getString("text");
                
                // Clean the response by removing system prompt references
                text = cleanResponseText(text);
                
                return new ChatResponse(text);
            }
        }
        
        throw new JSONException("No valid response found");
    }

    private static ChatResponse parseOpenAIResponse(String json) throws JSONException {
        JSONObject response = new JSONObject(json);
        JSONArray choices = response.getJSONArray("choices");
        
        if (choices.length() > 0) {
            JSONObject choice = choices.getJSONObject(0);
            JSONObject message = choice.getJSONObject("message");
            String text = message.getString("content");
            
            // Clean the response
            text = cleanResponseText(text);
            
            return new ChatResponse(text);
        }
        
        throw new JSONException("No valid response found");
    }

    private static ChatResponse parseAnthropicResponse(String json) throws JSONException {
        JSONObject response = new JSONObject(json);
        JSONArray content = response.getJSONArray("content");
        
        if (content.length() > 0) {
            JSONObject firstContent = content.getJSONObject(0);
            String text = firstContent.getString("text");
            
            // Clean the response
            text = cleanResponseText(text);
            
            return new ChatResponse(text);
        }
        
        throw new JSONException("No valid response found");
    }

    private static ChatResponse parseGenericResponse(String json) throws JSONException {
        JSONObject o = new JSONObject(json);
        String text = o.optString("text", "");
        if (text.isEmpty()) {
            text = o.optString("content", "");
        }
        return new ChatResponse(text);
    }

    private static String cleanResponseText(String text) {
        if (text == null) return "";
        
        // Remove common system prompt references
        text = text.replaceAll("(?i)system prompt:", "");
        text = text.replaceAll("(?i)system message:", "");
        text = text.replaceAll("(?i)assistant:", "");
        text = text.replaceAll("(?i)ai assistant:", "");
        text = text.replaceAll("(?i)here's my response:", "");
        text = text.replaceAll("(?i)here is my response:", "");
        text = text.replaceAll("(?i)as an ai assistant,", "");
        text = text.replaceAll("(?i)as an ai,", "");
        text = text.replaceAll("(?i)as a language model,", "");
        
        text = text.trim();
        
        return text;
    }
} 