// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import android.content.Context;
import android.util.Log;
import android.widget.Button;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;

import org.chromium.base.ThreadUtils;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.extensions.ExtensionInfo;
import org.chromium.chrome.browser.extensions.Extensions;
import org.chromium.chrome.browser.extensions.OpenExtensionsById;
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
            ThreadUtils.runOnUiThread(() -> {
                // Show error message via callback
                callback.onError(
                    "🔑 **API Key Not Configured**\n\n" +
                    "To use AI chat, please configure your API key in the Chat Extension.\n\n" +
                    "**Supported Providers:**\n" +
                    "• OpenAI (ChatGPT)\n" +
                    "• Google Gemini\n" +
                    "• Anthropic Claude\n" +
                    "Please add your API key to enable AI chat functionality."
                );
                // Also show button on activity
                showConfigurationButton(
                    "🔑 Configure API Key", 
                    "Chat Extension"
                );
            });
            return;
        }

        // Check if AI model URL is configured
        if (apiModelUrl == null || apiModelUrl.isEmpty()) {
            Log.e(TAG, "AI model URL is null or empty");
            ThreadUtils.runOnUiThread(() -> {
                // Show error message via callback
                callback.onError(
                    "🤖 **AI Model Not Configured**\n\n" +
                    "To use AI chat, please configure your AI model URL in the settings.\n\n" +
                    "Please add your AI model to enable AI chat functionality."
                );
                // Also show button on activity
                showConfigurationButton(
                    "🤖 Configure AI Model",
                    "Chat Extension"
                );
            });
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

    /**
     * Shows a configuration button on the AI chat bottom sheet
     */
    private void showConfigurationButton(String buttonText, String extensionName) {
        // This method will be called from the AI chat component
        // The button will be added to the chat interface by the calling component
        Log.d(TAG, "Configuration button requested: " + buttonText);
    }
    
    /**
     * Creates a configuration button for the AI chat interface
     */
    public Button createConfigurationButton(Context context, String buttonText) {
        Button configButton = new Button(context);
        configButton.setText(buttonText);
        configButton.setTextSize(14f);
        configButton.setTypeface(null, android.graphics.Typeface.BOLD);
        
        // Create enhanced orange-yellow gradient background
        GradientDrawable gradient = new GradientDrawable();
        gradient.setShape(GradientDrawable.RECTANGLE);
        gradient.setColors(new int[]{
            Color.parseColor("#FF6B35"), // Vibrant orange
            Color.parseColor("#F7931E"), // Bright orange
            Color.parseColor("#FFD700")  // Gold/yellow
        });
        gradient.setOrientation(GradientDrawable.Orientation.LEFT_RIGHT);
        gradient.setCornerRadius(28f); // More rounded corners
        gradient.setStroke(2, Color.parseColor("#E65100")); // Orange border
        
        configButton.setBackground(gradient);
        configButton.setTextColor(Color.WHITE);
        configButton.setPadding(40, 20, 40, 20); // More padding
        configButton.setElevation(12f); // Higher shadow
        configButton.setTranslationZ(4f); // Additional depth
        
        // Add ripple effect for better interaction
        configButton.setStateListAnimator(null);
        configButton.setClickable(true);
        configButton.setFocusable(true);
        
        // Set layout parameters for bottom positioning
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 
            LinearLayout.LayoutParams.WRAP_CONTENT
        );
        params.setMargins(16, 8, 16, 16); // Left, Top, Right, Bottom margins
        configButton.setLayoutParams(params);
        
        // Set click listener
        configButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Use the specific extension ID directly
                final String extensionId = "lhgoolpdddhhfahbnofaomjhfhfjfhop";
                
                try {
                    ChromeActivity activity = ChromeActivity.getChromeActivity();
                    
                    // Check if the extension is installed
                    boolean extensionExists = Extensions.getExtensionsInfo().stream()
                            .anyMatch(ext -> ext.getId().equals(extensionId));
                    
                    if (extensionExists) {
                        // If extension is installed, open it directly
                        OpenExtensionsById.openExtensionByIdNative(extensionId);
                    } else {
                        // If extension is not installed, navigate to flow-store in a new tab
                        if (activity.getActivityTab() != null) {
                            // Create a new tab instead of using the current one
                            activity.getTabCreator(false).createNewTab(
                                new org.chromium.content_public.browser.LoadUrlParams("wootzapp://flow-store"),
                                org.chromium.chrome.browser.tab.TabLaunchType.FROM_CHROME_UI,
                                activity.getActivityTab());
                        } else {
                            // Fallback to intent if no active tab
                            android.content.Intent intent = new android.content.Intent(android.content.Intent.ACTION_VIEW);
                            intent.setData(android.net.Uri.parse("wootzapp://flow-store"));
                            activity.startActivity(intent);
                        }
                    }
                    
                    // Hide the button after click (the chat component should handle this)
                    configButton.setVisibility(View.GONE);
                    
                } catch (ChromeActivity.ChromeActivityNotFoundException e) {
                    Log.e(TAG, "ChromeActivity not found: " + e);
                }
            }
        });
        
        return configButton;
    }
}