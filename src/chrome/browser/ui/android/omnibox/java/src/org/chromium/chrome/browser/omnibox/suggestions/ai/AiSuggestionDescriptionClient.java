// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions.ai;

import android.util.Log;

import org.chromium.base.ThreadUtils;
import org.chromium.chrome.browser.omnibox.suggestions.ai.AiConfig;
import org.chromium.chrome.browser.omnibox.suggestions.ai.AiModelClient;
import org.chromium.components.omnibox.AutocompleteMatch;
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

/** Client for generating AI descriptions for omnibox suggestions. */
public class AiSuggestionDescriptionClient {
    private static final String TAG = "AiSuggestionDescriptionClient";
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(2);
    
    private final NetworkTrafficAnnotationTag TRAFFIC_ANN = NetworkTrafficAnnotationTag.createComplete(
        "ai_suggestion_description_client",
        """
        semantics { sender: "AI Suggestion Description Client" }
        policy {
          cookies_allowed: NO
          policy_exception_justification: "No cookies used"
        }
        """);

    public interface Callback {
        void onSuccess(List<SuggestionDescription> descriptions);
        void onError(String error);
    }

    /** Represents a description for a suggestion. */
    public static class SuggestionDescription {
        public final int position;
        public final String description;
        
        public SuggestionDescription(int position, String description) {
            this.position = position;
            this.description = description;
        }
    }

    public AiSuggestionDescriptionClient() {}

    /**
     * Generate AI suggestions (both headers and descriptions) based on the current query.
     * @param query The current user query
     * @param count Number of suggestions to generate
     * @param callback Callback to receive the results
     */
    public void generateAiSuggestions(String query, int count, Callback callback) {
        String apiKey = AiConfig.getApiKey();
        String apiModelUrl = AiConfig.getAiModel();

        // Check if API key is configured
        if (apiKey == null || apiKey.isEmpty()) {
            Log.w(TAG, "API key not configured, skipping AI suggestions");
            ThreadUtils.runOnUiThread(() -> callback.onError("API key not configured"));
            return;
        }

        // Check if AI model URL is configured
        if (apiModelUrl == null || apiModelUrl.isEmpty()) {
            Log.w(TAG, "AI model URL not configured, skipping AI suggestions");
            ThreadUtils.runOnUiThread(() -> callback.onError("AI model URL not configured"));
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

                // Create request body for AI suggestions
                String requestBody = createRequestForAiSuggestions(query, count, provider);
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

                List<SuggestionDescription> suggestions = parseAiSuggestionsResponse(responseString, provider, count);
                
                ThreadUtils.runOnUiThread(() -> {
                    callback.onSuccess(suggestions);
                });

            } catch (Exception e) {
                Log.e(TAG, "Error generating AI suggestions", e);
                ThreadUtils.runOnUiThread(() -> {
                    callback.onError("Failed to generate AI suggestions: " + e.getMessage());
                });
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        });
    }

    /**
     * Generate AI descriptions for the given suggestions.
     * @param suggestions List of suggestions to generate descriptions for
     * @param callback Callback to receive the results
     */
    public void generateDescriptions(List<AutocompleteMatch> suggestions, Callback callback) {
        String apiKey = AiConfig.getApiKey();
        String apiModelUrl = AiConfig.getAiModel();

        // Check if API key is configured
        if (apiKey == null || apiKey.isEmpty()) {
            Log.w(TAG, "API key not configured, skipping AI descriptions");
            ThreadUtils.runOnUiThread(() -> callback.onError("API key not configured"));
            return;
        }

        // Check if AI model URL is configured
        if (apiModelUrl == null || apiModelUrl.isEmpty()) {
            Log.w(TAG, "AI model URL not configured, skipping AI descriptions");
            ThreadUtils.runOnUiThread(() -> callback.onError("AI model URL not configured"));
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
                String requestBody = createRequestForSuggestions(suggestions, provider);
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

                List<SuggestionDescription> descriptions = parseResponse(responseString, provider);
                
                ThreadUtils.runOnUiThread(() -> {
                    callback.onSuccess(descriptions);
                });

            } catch (Exception e) {
                Log.e(TAG, "Error generating AI descriptions", e);
                ThreadUtils.runOnUiThread(() -> {
                    callback.onError("Failed to generate descriptions: " + e.getMessage());
                });
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        });
    }

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

    private String createRequestForAiSuggestions(String query, int count, String provider) throws JSONException {
        StringBuilder prompt = new StringBuilder();
        prompt.append("Generate ").append(count).append(" browser omnibox suggestions for query: \"").append(query).append("\"\n\n");
        prompt.append("CRITICAL: Return ONLY a valid JSON array with ").append(count * 2).append(" strings.\n");
        prompt.append("Format: [\"header1\", \"description1\", \"header2\", \"description2\", ...]\n\n");
        prompt.append("Rules:\n");
        prompt.append("- Headers: 1-3 words only (examples: \"weather\", \"maps\", \"translate\")\n");
        prompt.append("- Descriptions: One short sentence (examples: \"Check weather forecast\", \"Find directions\")\n");
        prompt.append("- NO quotes around individual items in JSON\n");
        prompt.append("- NO markdown formatting\n");
        prompt.append("- NO extra text or explanations\n\n");
        prompt.append("- Think like Google's omnibox suggestions\n\n");
        prompt.append("- Make suggestions relevant, diverse, and helpful for the user's search intent.");

        switch (provider) {
            case "GEMINI":
                return createGeminiRequest(prompt.toString());
            case "OPENAI":
                return createOpenAIRequest(prompt.toString());
            case "ANTHROPIC":
                return createAnthropicRequest(prompt.toString());
            default:
                return createGenericRequest(prompt.toString());
        }
    }

    private String createRequestForSuggestions(List<AutocompleteMatch> suggestions, String provider) throws JSONException {
        StringBuilder prompt = new StringBuilder();
        prompt.append("Generate brief, helpful descriptions for these search suggestions. ");
        prompt.append("Each description should be 1-2 sentences explaining what the user would find. ");
        prompt.append("Return ONLY a JSON array with descriptions in the same order.\n\n");
        
        for (int i = 0; i < suggestions.size(); i++) {
            AutocompleteMatch suggestion = suggestions.get(i);
            prompt.append(i + 1).append(". ").append(suggestion.getDisplayText());
            if (suggestion.getUrl() != null && !suggestion.getUrl().getSpec().isEmpty()) {
                prompt.append(" (URL: ").append(suggestion.getUrl().getSpec()).append(")");
            }
            prompt.append("\n");
        }
        
        prompt.append("\nReturn format: [\"description1\", \"description2\", ...]");

        switch (provider) {
            case "GEMINI":
                return createGeminiRequest(prompt.toString());
            case "OPENAI":
                return createOpenAIRequest(prompt.toString());
            case "ANTHROPIC":
                return createAnthropicRequest(prompt.toString());
            default:
                return createGenericRequest(prompt.toString());
        }
    }

    private String createGeminiRequest(String prompt) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray contents = new JSONArray();
        
        JSONObject content = new JSONObject()
                .put("role", "user")
                .put("parts", new JSONArray()
                        .put(new JSONObject().put("text", prompt)));
        contents.put(content);
        
        request.put("contents", contents);
        request.put("model", "gemini-2.5-flash");
        
        return request.toString();
    }

    private String createOpenAIRequest(String prompt) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray messages = new JSONArray();
        
        JSONObject message = new JSONObject()
                .put("role", "user")
                .put("content", prompt);
        messages.put(message);
        
        request.put("model", "gpt-4o");
        request.put("messages", messages);
        
        return request.toString();
    }

    private String createAnthropicRequest(String prompt) throws JSONException {
        JSONObject request = new JSONObject();
        JSONArray messages = new JSONArray();
        
        JSONObject message = new JSONObject()
                .put("role", "user")
                .put("content", prompt);
        messages.put(message);
        
        request.put("model", "claude-4");
        request.put("max_tokens", 1024);
        request.put("messages", messages);
        
        return request.toString();
    }

    private String createGenericRequest(String prompt) throws JSONException {
        JSONObject request = new JSONObject();
        request.put("message", prompt);
        return request.toString();
    }

    private List<SuggestionDescription> parseResponse(String responseString, String provider) throws JSONException {
        List<SuggestionDescription> descriptions = new ArrayList<>();
        
        switch (provider) {
            case "GEMINI":
                descriptions = parseGeminiResponse(responseString);
                break;
            case "OPENAI":
                descriptions = parseOpenAIResponse(responseString);
                break;
            case "ANTHROPIC":
                descriptions = parseAnthropicResponse(responseString);
                break;
            default:
                descriptions = parseGenericResponse(responseString);
                break;
        }
        
        return descriptions;
    }

    private List<SuggestionDescription> parseGeminiResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray candidates = response.getJSONArray("candidates");
        
        if (candidates.length() > 0) {
            JSONObject candidate = candidates.getJSONObject(0);
            JSONObject content = candidate.getJSONObject("content");
            JSONArray parts = content.getJSONArray("parts");
            
            if (parts.length() > 0) {
                JSONObject part = parts.getJSONObject(0);
                String text = part.getString("text");
                return parseDescriptionsFromText(text);
            }
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseOpenAIResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray choices = response.getJSONArray("choices");
        
        if (choices.length() > 0) {
            JSONObject choice = choices.getJSONObject(0);
            JSONObject message = choice.getJSONObject("message");
            String text = message.getString("content");
            return parseDescriptionsFromText(text);
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseAnthropicResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray content = response.getJSONArray("content");
        
        if (content.length() > 0) {
            JSONObject firstContent = content.getJSONObject(0);
            String text = firstContent.getString("text");
            return parseDescriptionsFromText(text);
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseGenericResponse(String responseString) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        String text = response.optString("text", "");
        if (text.isEmpty()) {
            text = response.optString("content", "");
        }
        return parseDescriptionsFromText(text);
    }

    private List<SuggestionDescription> parseAiSuggestionsResponse(String responseString, String provider, int count) throws JSONException {
        List<SuggestionDescription> suggestions = new ArrayList<>();
        
        switch (provider) {
            case "GEMINI":
                suggestions = parseGeminiAiSuggestionsResponse(responseString, count);
                break;
            case "OPENAI":
                suggestions = parseOpenAIAiSuggestionsResponse(responseString, count);
                break;
            case "ANTHROPIC":
                suggestions = parseAnthropicAiSuggestionsResponse(responseString, count);
                break;
            default:
                suggestions = parseGenericAiSuggestionsResponse(responseString, count);
                break;
        }
        
        return suggestions;
    }

    private List<SuggestionDescription> parseGeminiAiSuggestionsResponse(String responseString, int count) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray candidates = response.getJSONArray("candidates");
        
        if (candidates.length() > 0) {
            JSONObject candidate = candidates.getJSONObject(0);
            JSONObject content = candidate.getJSONObject("content");
            JSONArray parts = content.getJSONArray("parts");
            
            if (parts.length() > 0) {
                JSONObject part = parts.getJSONObject(0);
                String text = part.getString("text");
                return parseAiSuggestionsFromText(text, count);
            }
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseOpenAIAiSuggestionsResponse(String responseString, int count) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray choices = response.getJSONArray("choices");
        
        if (choices.length() > 0) {
            JSONObject choice = choices.getJSONObject(0);
            JSONObject message = choice.getJSONObject("message");
            String text = message.getString("content");
            return parseAiSuggestionsFromText(text, count);
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseAnthropicAiSuggestionsResponse(String responseString, int count) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        JSONArray content = response.getJSONArray("content");
        
        if (content.length() > 0) {
            JSONObject firstContent = content.getJSONObject(0);
            String text = firstContent.getString("text");
            return parseAiSuggestionsFromText(text, count);
        }
        
        throw new JSONException("No valid response found");
    }

    private List<SuggestionDescription> parseGenericAiSuggestionsResponse(String responseString, int count) throws JSONException {
        JSONObject response = new JSONObject(responseString);
        String text = response.optString("text", "");
        if (text.isEmpty()) {
            text = response.optString("content", "");
        }
        return parseAiSuggestionsFromText(text, count);
    }

    private List<SuggestionDescription> parseAiSuggestionsFromText(String text, int count) throws JSONException {
        List<SuggestionDescription> suggestions = new ArrayList<>();
        
        // Clean the text first - remove markdown formatting and extra whitespace
        String cleanText = text.trim()
                .replaceAll("```json", "")
                .replaceAll("```", "")
                .replaceAll("\\*\\*", "")
                .replaceAll("\\*", "")
                .trim();
        
        // Try to parse as JSON array first
        try {
            JSONArray jsonArray = new JSONArray(cleanText);
            int maxItems = Math.min(jsonArray.length(), count * 2);
            for (int i = 0; i < maxItems; i++) {
                String item = jsonArray.getString(i).trim();
                // Remove quotes if present
                if (item.startsWith("\"") && item.endsWith("\"")) {
                    item = item.substring(1, item.length() - 1);
                }
                suggestions.add(new SuggestionDescription(i, item));
            }
        } catch (JSONException e) {
            // If not JSON, try to extract suggestions from text
            String[] lines = cleanText.split("\n");
            int itemIndex = 0;
            for (String line : lines) {
                if (itemIndex >= count * 2) break;
                String trimmedLine = line.trim();
                if (!trimmedLine.isEmpty() && !trimmedLine.startsWith("```") && !trimmedLine.startsWith("**")) {
                    // Remove quotes if present
                    if (trimmedLine.startsWith("\"") && trimmedLine.endsWith("\"")) {
                        trimmedLine = trimmedLine.substring(1, trimmedLine.length() - 1);
                    }
                    suggestions.add(new SuggestionDescription(itemIndex, trimmedLine));
                    itemIndex++;
                }
            }
        }
        
        return suggestions;
    }

    private List<SuggestionDescription> parseDescriptionsFromText(String text) throws JSONException {
        List<SuggestionDescription> descriptions = new ArrayList<>();
        
        // Try to parse as JSON array first
        try {
            JSONArray jsonArray = new JSONArray(text);
            for (int i = 0; i < jsonArray.length(); i++) {
                String description = jsonArray.getString(i);
                descriptions.add(new SuggestionDescription(i, description));
            }
        } catch (JSONException e) {
            // If not JSON, try to extract descriptions from text
            String[] lines = text.split("\n");
            for (int i = 0; i < lines.length; i++) {
                String line = lines[i].trim();
                if (!line.isEmpty() && !line.startsWith("```") && !line.startsWith("**")) {
                    descriptions.add(new SuggestionDescription(i, line));
                }
            }
        }
        
        return descriptions;
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
