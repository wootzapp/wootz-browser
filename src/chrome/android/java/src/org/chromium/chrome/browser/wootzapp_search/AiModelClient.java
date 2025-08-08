// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

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

/** Client for communicating with AiModel API. */
public class AiModelClient {
    private static final String TAG = "AiModelClient";
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(3);
    
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
            ThreadUtils.runOnUiThread(() -> callback.onError(
                "🔑 **API Key Not Configured**\n\n" +
                "To use AI chat, please configure your API key in the Chat Extension.\n\n" +
                "**Supported Providers:**\n" +
                "• OpenAI (ChatGPT)\n" +
                "• Google Gemini\n" +
                "• Anthropic Claude\n" +
                "Please add your API key to enable AI chat functionality."
            ));
            return;
        }

        // Check if AI model URL is configured
        if (apiModelUrl == null || apiModelUrl.isEmpty()) {
            Log.e(TAG, "AI model URL is null or empty");
            ThreadUtils.runOnUiThread(() -> callback.onError(
                "🤖 **AI Model Not Configured**\n\n" +
                "To use AI chat, please configure your AI model URL in the settings.\n\n" +
                "Please add your AI model to enable AI chat functionality."
            ));
            return;
        }

        // Validate URL format
        try {
            new URL(apiModelUrl);
        } catch (Exception e) {
            Log.e(TAG, "Invalid AI model URL: " + apiModelUrl);
            ThreadUtils.runOnUiThread(() -> callback.onError(
                " **Invalid AI Model URL**\n\n" +
                "The configured AI model URL is not valid:\n" +
                "`" + apiModelUrl + "`\n\n" +
                "Please check your AI model URL configuration and try again."
            ));
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
                    // For other providers, use the base URL directly
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
                        Log.w(TAG, "Unknown provider: " + provider + ", using Authorization header as fallback");
                        connection.setRequestProperty("Authorization", "Bearer " + apiKey);
                        break;
                }

                Log.e(TAG, "HTTP connection configured for provider: " + provider);

                // Create request using ChatRequest
                List<ChatMessage> messages = new ArrayList<>();
                messages.add(new ChatMessage(message, true));
                ChatRequest request = new ChatRequest(messages, provider);
                String body = request.toJson().toString();
                Log.e(TAG, "Request body: " + body);

                try (OutputStream os = connection.getOutputStream()) {
                    os.write(body.getBytes(StandardCharsets.UTF_8));
                }

                int responseCode = connection.getResponseCode();

                InputStream inputStream = (responseCode < 400) ? connection.getInputStream() : connection.getErrorStream();
                String responseString = readStream(inputStream);

                if (responseCode >= 400) {
                    String errorMessage = getErrorMessage(responseCode, responseString, provider);
                    throw new IOException(errorMessage);
                }

                ChatResponse response = ChatResponse.fromJson(responseString, provider);
                String result = response.getText();
                
                
                ThreadUtils.runOnUiThread(() -> {
                    callback.onSuccess(result);
                });

            } catch (Exception e) {
                ThreadUtils.runOnUiThread(() -> {
                    callback.onError("❌ **AI Service Error**\n\n" + e.getMessage());
                });
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        });
    }

    /**
     * Provides user-friendly error messages based on HTTP response codes
     */
    private String getErrorMessage(int responseCode, String responseString, String provider) {
        switch (responseCode) {
            case 401:
                return "🔐 **Authentication Failed**\n\n" +
                       "Your API key is invalid or has expired. Please check your API key configuration.";
            case 403:
                return " **Access Forbidden**\n\n" +
                       "Your API key doesn't have permission to access this service. Please check your API key permissions.";
            case 404:
                return "🔍 **Model Not Found**\n\n" +
                       "The AI model specified in the URL was not found. Please check your model configuration.";
            case 429:
                return "⏱️ **Rate Limit Exceeded**\n\n" +
                       "You've exceeded the rate limit for this AI service. Please try again later.";
            case 500:
                return " **Server Error**\n\n" +
                       "The AI service is experiencing technical difficulties. Please try again later.";
            case 502:
            case 503:
            case 504:
                return "🌐 **Service Unavailable**\n\n" +
                       "The AI service is temporarily unavailable. Please try again later.";
            default:
                return "HTTP " + responseCode + ": " + responseString;
        }
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