// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;

/**
 * Formats conversation history for different AI providers.
 */
public class ChatRequest {

    private final List<ChatMessage> mMessages;
    private final String mProvider;

    public ChatRequest(List<ChatMessage> messages, String provider) {
        this.mMessages = messages;
        this.mProvider = provider;
    }

    public JSONObject toJson() throws JSONException {
        switch (mProvider) {
            case "GEMINI":
                return createGeminiRequest();
            case "OPENAI":
                return createOpenAIRequest();
            case "ANTHROPIC":
                return createAnthropicRequest();
            default:
                return createGenericRequest();
        }
    }

    private JSONObject createGeminiRequest() throws JSONException {
        JSONArray contents = new JSONArray();
        
        JSONObject systemContent = new JSONObject()
                .put("role", "model")
                .put("parts", new JSONArray()
                        .put(new JSONObject().put("text", 
                            "You are a search-based AI assistant. Treat EVERY user input as a search query, even if it's just a word like 'hello' or 'test'. " +
                            "For any input, search across multiple sources, websites, and search engines to provide comprehensive information. " +
                            "Always provide factual, up-to-date information based on search results. " +
                            "For greetings like 'hello', search for its meaning, history, usage, or cultural significance. " +
                            "For single words, search for their definitions, related information, and current context. " +
                            "Provide search-based responses with relevant facts, data, and information from reliable sources.")));
        contents.put(systemContent);
        
        // Add conversation messages
        for (ChatMessage m : mMessages) {
            JSONObject content = new JSONObject()
                    .put("role", m.isUser() ? "user" : "model")
                    .put("parts", new JSONArray()
                            .put(new JSONObject().put("text", m.getMessage())));
            contents.put(content);
        }
        
        return new JSONObject()
                .put("contents", contents)
                .put("model", "gemini-2.5-flash");
    }

    private JSONObject createOpenAIRequest() throws JSONException {
        JSONArray messages = new JSONArray();
        
        JSONObject systemMessage = new JSONObject()
                .put("role", "system")
                .put("content", 
                    "You are a search-based AI assistant. Treat EVERY user input as a search query, even if it's just a word like 'hello' or 'test'. " +
                    "For any input, search across multiple sources, websites, and search engines to provide comprehensive information. " +
                    "Always provide factual, up-to-date information based on search results. " +
                    "For greetings like 'hello', search for its meaning, history, usage, or cultural significance. " +
                    "For single words, search for their definitions, related information, and current context. " +
                    "Provide search-based responses with relevant facts, data, and information from reliable sources.");
        messages.put(systemMessage);
        
        // Add user messages
        for (ChatMessage m : mMessages) {
            if (m.isUser()) {
                JSONObject message = new JSONObject()
                        .put("role", "user")
                        .put("content", m.getMessage());
                messages.put(message);
            }
        }
        
        return new JSONObject()
                .put("model", "gpt-4o")
                .put("messages", messages);
    }

    private JSONObject createAnthropicRequest() throws JSONException {
        JSONArray messages = new JSONArray();
        
        JSONObject systemMessage = new JSONObject()
                .put("role", "system")
                .put("content", 
                    "You are a search-based AI assistant. Treat EVERY user input as a search query, even if it's just a word like 'hello' or 'test'. " +
                    "For any input, search across multiple sources, websites, and search engines to provide comprehensive information. " +
                    "Always provide factual, up-to-date information based on search results. " +
                    "For greetings like 'hello', search for its meaning, history, usage, or cultural significance. " +
                    "For single words, search for their definitions, related information, and current context. " +
                    "Provide search-based responses with relevant facts, data, and information from reliable sources.");
        messages.put(systemMessage);
        
        // Add user messages
        for (ChatMessage m : mMessages) {
            if (m.isUser()) {
                JSONObject message = new JSONObject()
                        .put("role", "user")
                        .put("content", m.getMessage());
                messages.put(message);
            }
        }
        
        return new JSONObject()
                .put("model", "claude-4")
                .put("max_tokens", 1024)
                .put("messages", messages);
    }

    private JSONObject createGenericRequest() throws JSONException {
        if (mMessages.isEmpty()) {
            return new JSONObject().put("message", "");
        }
        
        String systemPrompt = 
            "You are a search-based AI assistant. Treat EVERY user input as a search query, even if it's just a word like 'hello' or 'test'. " +
            "For any input, search across multiple sources, websites, and search engines to provide comprehensive information. " +
            "Always provide factual, up-to-date information based on search results. " +
            "For greetings like 'hello', search for its meaning, history, usage, or cultural significance. " +
            "For single words, search for their definitions, related information, and current context. " +
            "Provide search-based responses with relevant facts, data, and information from reliable sources.\n\n" +
            "Search query: ";
        String fullContent = systemPrompt + mMessages.get(mMessages.size() - 1).getMessage();
        
        return new JSONObject().put("message", fullContent);
    }
} 