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

/** Client for communicating with Gemini API. */
public class GeminiApiClient {
    private static final String TAG = "GeminiApiClient";
    private static final String GEMINI_API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-pro-latest:generateContent";
    
    private final String mApiKey;
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(3);
    
    private final NetworkTrafficAnnotationTag TRAFFIC_ANN = NetworkTrafficAnnotationTag.createComplete(
            "gemini_api_client",
            """
            semantics { sender: "Gemini API Client" }
            policy {
              cookies_allowed: NO
              policy_exception_justification: "No cookies used"
            }
            """);

    public interface Callback {
        void onSuccess(String response);
        void onError(String error);
    }

    public GeminiApiClient(String apiKey) {
        // Use the provided API key or fallback to the hardcoded one
        mApiKey = apiKey;
        Log.d(TAG, "GeminiApiClient created with API key: " + (mApiKey != null ? "present" : "null"));
    }

    public void sendMessage(String message, Callback callback) {
        Log.d(TAG, "sendMessage called with message: '" + message + "'");
        
        // Create a simple conversation with just the user message
        List<ChatMessage> messages = new ArrayList<>();
        messages.add(new ChatMessage(message, true));
        
        // Create request
        ChatRequest request = new ChatRequest(messages);
        
        mExecutor.submit(() -> {
            HttpURLConnection connection = null;
            try {
                String urlWithKey = GEMINI_API_URL + "?key=" + mApiKey;
                URL url = new URL(urlWithKey);
                
                // Use Chromium's network adapter
                connection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(url, TRAFFIC_ANN);
                
                connection.setRequestMethod("POST");
                connection.setDoOutput(true);
                connection.setConnectTimeout(10_000);
                connection.setReadTimeout(30_000);
                connection.setRequestProperty("Content-Type", "application/json");
                
                Log.d(TAG, "HTTP connection configured");

                // Create request body using ChatRequest
                String body = request.toJson().toString();
                Log.d(TAG, "Request body: " + body);

                // Send request
                try (OutputStream os = connection.getOutputStream()) {
                    os.write(body.getBytes(StandardCharsets.UTF_8));
                }

                // Check response code
                int responseCode = connection.getResponseCode();
                Log.d(TAG, "Response code: " + responseCode);

                // Read response
                InputStream inputStream = (responseCode < 400) ? connection.getInputStream() : connection.getErrorStream();
                String responseString = readStream(inputStream);
                Log.d(TAG, "Raw response: " + responseString);

                if (responseCode >= 400) {
                    throw new IOException("HTTP " + responseCode + ": " + responseString);
                }

                // Parse response using ChatResponse
                ChatResponse response = ChatResponse.fromJson(responseString);
                String result = response.getText();
                
                Log.d(TAG, "Parsed result: " + result);
                
                // Return success on UI thread
                ThreadUtils.runOnUiThread(() -> {
                    Log.d(TAG, "Calling onSuccess callback with result: " + result);
                    callback.onSuccess(result);
                });

            } catch (Exception e) {
                Log.e(TAG, "Error calling Gemini API", e);
                // Return error on UI thread
                ThreadUtils.runOnUiThread(() -> {
                    Log.d(TAG, "Calling onError callback with error: " + e.getMessage());
                    callback.onError("Failed to get response from AI: " + e.getMessage());
                });
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        });
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