// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions.ai;

import android.util.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.net.ChromiumNetworkAdapter;
import org.chromium.net.NetworkTrafficAnnotationTag;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/** Client for communicating with AI model API for suggestion descriptions. */
public class AiModelClient {
    private static final String TAG = "AiModelClient";
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(2);
    
    private final NetworkTrafficAnnotationTag TRAFFIC_ANN = NetworkTrafficAnnotationTag.createComplete(
        "ai_model_api_client",
        """
        semantics { sender: "AI Model API Client" }
        policy {
          cookies_allowed: NO
          policy_exception_justification: "No cookies used"
        }
        """);

    public interface Callback {
        void onSuccess(String response);
        void onError(String error);
    }

    public AiModelClient() { }

    private String getProviderFromUrl(String url) {
        if (url.contains("generativelanguage.googleapis.com")) {
            return "GEMINI";
        } else if (url.contains("api.openai.com")) {
            return "OPENAI";
        } else if (url.contains("api.anthropic.com")) {
            return "ANTHROPIC";
        }
        return "UNKNOWN";
    }

    public void sendMessage(String message, Callback callback) {
        String apiKey = AiConfig.getApiKey();
        String apiModelUrl = AiConfig.getAiModel();

        // Check if API key is configured
        if (apiKey == null || apiKey.isEmpty()) {
            Log.e(TAG, "API key is null or empty");
            ThreadUtils.runOnUiThread(() -> {
                callback.onError("API Key Not Configured");
            });
            return;
        }

        // Check if AI model URL is configured
        if (apiModelUrl == null || apiModelUrl.isEmpty()) {
            Log.e(TAG, "AI model URL is null or empty");
            ThreadUtils.runOnUiThread(() -> {
                callback.onError("AI Model Not Configured");
            });
            return;
        }

        // Validate URL format
        try {
            new URL(apiModelUrl);
        } catch (Exception e) {
            Log.e(TAG, "Invalid AI model URL: " + apiModelUrl);
            ThreadUtils.runOnUiThread(() -> callback.onError("Invalid AI Model URL"));
            return;
        }
        
        mExecutor.submit(() -> {
            HttpURLConnection connection = null;
            try {
                String provider = getProviderFromUrl(apiModelUrl);
                URL url;

                if ("GEMINI".equals(provider)) {
                    String urlWithKey = apiModelUrl + "?key=" + apiKey;
                    url = new URL(urlWithKey);
                } else {
                    url = new URL(apiModelUrl);
                }
                
                connection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(url, TRAFFIC_ANN);
                connection.setRequestMethod("POST");
                connection.setDoOutput(true);
                connection.setConnectTimeout(10_000);
                connection.setReadTimeout(30_000);
                connection.setRequestProperty("Content-Type", "application/json");
                
                // Set authentication header based on provider
                switch (provider) {
                    case "OPENAI":
                        connection.setRequestProperty("Authorization", "Bearer " + apiKey);
                        break;
                    case "ANTHROPIC":
                        connection.setRequestProperty("x-api-key", apiKey);
                        connection.setRequestProperty("anthropic-version", "2023-06-01");
                        break;
                    case "GEMINI":
                        // Already handled in URL
                        break;
                    default:
                        connection.setRequestProperty("Authorization", "Bearer " + apiKey);
                        break;
                }

                // Create request body
                String requestBody = createRequestForMessage(message, provider);
                Log.d(TAG, "Request body: " + requestBody);

                try (OutputStream os = connection.getOutputStream()) {
                    os.write(requestBody.getBytes(StandardCharsets.UTF_8));
                }

                int responseCode = connection.getResponseCode();
                InputStream inputStream = (responseCode < 400) ? connection.getInputStream() : connection.getErrorStream();
                String responseString = readStream(inputStream);

                if (responseCode >= 400) {
                    throw new IOException("HTTP " + responseCode + ": " + responseString);
                }

                String result = parseResponse(responseString, provider);
                
                ThreadUtils.runOnUiThread(() -> {
                    callback.onSuccess(result);
                });

            } catch (Exception e) {
                Log.e(TAG, "Error in AI API call", e);
                ThreadUtils.runOnUiThread(() -> {
                    callback.onError("AI Service Error: " + e.getMessage());
                });
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        });
    }

    private String createRequestForMessage(String message, String provider) throws JSONException {
        switch (provider) {
            case "GEMINI":
                return createGeminiRequest(message);
            case "OPENAI":
                return createOpenAIRequest(message);
            case "ANTHROPIC":
                return createAnthropicRequest(message);
            default:
                return createGenericRequest(message);
        }
    }

    private String createGeminiRequest(String message) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray contents = new JSONArray();
        
        JSONObject content = new JSONObject()
                .put("role", "user")
                .put("parts", new JSONArray()
                        .put(new JSONObject().put("text", message)));
        contents.put(content);
        
        request.put("contents", contents);
        request.put("model", "gemini-2.5-flash");
        
        return request.toString();
    }

    private String createOpenAIRequest(String message) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray messages = new JSONArray();
        
        JSONObject messageObj = new JSONObject()
                .put("role", "user")
                .put("content", message);
        messages.put(messageObj);
        
        request.put("model", "gpt-4o");
        request.put("messages", messages);
        
        return request.toString();
    }

    private String createAnthropicRequest(String message) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray messages = new JSONArray();
        
        JSONObject messageObj = new JSONObject()
                .put("role", "user")
                .put("content", message);
        messages.put(messageObj);
        
        request.put("model", "claude-4");
        request.put("max_tokens", 1024);
        request.put("messages", messages);
        
        return request.toString();
    }

    private String createGenericRequest(String message) throws JSONException {
        JSONObject request = new JSONObject();
        request.put("message", message);
        return request.toString();
    }

    private String parseResponse(String responseString, String provider) throws JSONException {
        switch (provider) {
            case "GEMINI":
                return parseGeminiResponse(responseString);
            case "OPENAI":
                return parseOpenAIResponse(responseString);
            case "ANTHROPIC":
                return parseAnthropicResponse(responseString);
            default:
                return parseGenericResponse(responseString);
        }
    }

    private String parseGeminiResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray candidates = response.getJSONArray("candidates");
        
        if (candidates.length() > 0) {
            JSONObject candidate = candidates.getJSONObject(0);
            JSONObject content = candidate.getJSONObject("content");
            JSONArray parts = content.getJSONArray("parts");
            
            if (parts.length() > 0) {
                JSONObject part = parts.getJSONObject(0);
                return part.getString("text");
            }
        }
        
        throw new JSONException("No valid response found");
    }

    private String parseOpenAIResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray choices = response.getJSONArray("choices");
        
        if (choices.length() > 0) {
            JSONObject choice = choices.getJSONObject(0);
            JSONObject message = choice.getJSONObject("message");
            return message.getString("content");
        }
        
        throw new JSONException("No valid response found");
    }

    private String parseAnthropicResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray content = response.getJSONArray("content");
        
        if (content.length() > 0) {
            JSONObject firstContent = content.getJSONObject(0);
            return firstContent.getString("text");
        }
        
        throw new JSONException("No valid response found");
    }

    private String parseGenericResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        String text = response.optString("text", "");
        if (text.isEmpty()) {
            text = response.optString("content", "");
        }
        return text;
    }

    private static String readStream(InputStream in) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = reader.readLine()) != null) {
            sb.append(line);
        }
        return sb.toString();
    }
}
