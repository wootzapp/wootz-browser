package org.chromium.components.action_url;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import org.json.JSONArray;
import org.json.JSONObject;
import java.util.HashMap;
import java.util.Map;
import android.util.Log;
import org.chromium.net.ChromiumNetworkAdapter;
import org.chromium.net.NetworkTrafficAnnotationTag;
import java.nio.charset.StandardCharsets;
import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;
import java.util.HashSet;
import java.util.Set;
import org.chromium.base.ThreadUtils;
import android.util.Pair;
import java.util.List;
import java.util.Collections;
import java.util.ArrayList;

// @JNINamespace("action_url")
public class ActionUrlListFetcher{

    private static ActionUrlListFetcher mInstance;
    private static Map<String, Pair<String, String>> mActionUrlMap;
    private boolean actionListFetched;

    final Object mLock = new Object();

    NetworkTrafficAnnotationTag TRAFFIC_ANNOTATION = NetworkTrafficAnnotationTag.createComplete(
        "action_url_fetcher",
    """
                semantics {
                  sender: "Action Url Fetcher"
                }
                policy {
                  cookies_allowed: NO
                  policy_exception_justification: "Not implemented."
                }
        """);
    private ActionUrlListFetcher() {
        mActionUrlMap = new HashMap<>();
        fetchActionUrlList();
    }

    private String formatUrl(String url) {
        if(url.endsWith("/")) {
            return url.substring(0,url.lastIndexOf("/"));
        }
        return url;
    }

    // Method to get the singleton instance
    public static ActionUrlListFetcher getInstance() {
        if (mInstance == null) {
            mInstance = new ActionUrlListFetcher();
        }
        return mInstance;
    }

    // @CalledByNative
    public void processFetchedContent(String jsonString) {
        try {
            // Parse the JSON string to a JSONObject
            JSONObject jsonObject = new JSONObject(jsonString);
            JSONArray resultsArray = jsonObject.getJSONArray("results");
            Log.e("::ACTION_URL:: kritagya", "resultsArray:: " + resultsArray.length());
            // Populate the HashMap with blinkUrl as key and actionUrl as value
            for (int i = 0; i < resultsArray.length(); i++) {
                JSONObject resultObject = resultsArray.getJSONObject(i);
                String blinkUrl = resultObject.getString("blinkUrl");
                String actionUrl = resultObject.getString("actionUrl");
                String websiteUrl = resultObject.getString("websiteUrl");
                JSONArray tags = resultObject.getJSONArray("tags");
                String tag;
                Log.e("::ACTION_URL:: kritagya", "tags:: " + tags.toString() + "||| length:: " + tags.length());
                Log.e("::ACTION_URL:: kritagya", "blinkUrl:: " + blinkUrl);
                Log.e("::ACTION_URL:: kritagya", "actionUrl:: " + actionUrl);
                Log.e("::ACTION_URL:: kritagya", "websiteUrl:: " + websiteUrl);
                if(tags.length() == 0) {
                    tag = "unknown";
                }
                else{
                    tag = tags.getString(0);
                }

                if(!blinkUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(blinkUrl), new Pair<>(actionUrl, tag));
                    // Log.e("::ACTION_URL::", "blinkUrl:: " + blinkUrl + "||| actionUrl:: " + actionUrl);
                }
                if(!websiteUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(websiteUrl), new Pair<>(actionUrl, tag));
                    // Log.e("::ACTION_URL::", "websiteUrl:: " + websiteUrl + "||| actionUrl:: " + actionUrl);
                }

                mActionUrlMap.put(formatUrl(actionUrl), new Pair<>(actionUrl, tag));

            }
        } catch (Exception e) {
            Log.e("ActionUrl::", "Error parsing JSON: " + e.getMessage());
        }
    }

    private void expandToFinalURL(String shortUrl,  ActionUrlFetchedCallback callback) {
        HttpURLConnection httpURLConnection = null;
        String expandedURL = shortUrl;
        Log.e("::Unfurling Sagar:::: abcdz ", "expandUrl url is: "+shortUrl);
        try {
            URL url = new URL(shortUrl);
            httpURLConnection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(url,
                    TRAFFIC_ANNOTATION);

            httpURLConnection.setConnectTimeout(5000); // 5 seconds timeout
            httpURLConnection.setReadTimeout(5000); // 5 seconds read timeout
            // Use the HEAD method to reduce overhead
            httpURLConnection.setRequestMethod("HEAD");
            httpURLConnection.setInstanceFollowRedirects(false);

            // Connect to the URL
            httpURLConnection.connect();

            // Check for redirection
            int responseCode = httpURLConnection.getResponseCode();
            Log.e("::Unfurling Sagar:::: ", "Response code is: " + responseCode);
            if (responseCode == HttpURLConnection.HTTP_MOVED_PERM || 
                responseCode == HttpURLConnection.HTTP_MOVED_TEMP || 
                responseCode == HttpURLConnection.HTTP_SEE_OTHER) {
                // Get the "Location" header to find the redirect target
                expandedURL = httpURLConnection.getHeaderField("Location");  
            } else {
                // If there's no redirection, return the original URL
                expandedURL = shortUrl;
            }
            Log.e("::Unfurling Sagar:::: abcdz ", "expandedURL is: " + expandedURL);

        } catch (Exception e) {
            Log.e("::Unfurling Sagar:::: abcdz ", "Failed to expand blink url: " + e.getMessage());
        } finally {
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
            }
        }

        Log.e("::Unfurling Sagar:::: abcdz ", "expandedURL is: " + expandedURL);

        Pair<String, String> action_pair = mActionUrlMap.get(formatUrl(expandedURL));
        String twitterActionUrl = processUrlForActionJson(expandedURL);
        Log.e("::Unfurling Sagar:::: abcdz ", "twitterActionUrl is: " + twitterActionUrl);

        if (action_pair != null) {
            Log.e("::Unfurling Sagar:::: abcdz ", "action_pair first is: " + action_pair.first);
            Log.e("::Unfurling Sagar:::: abcdz ", "action_pair second is: " + action_pair.second);
        } else {
            Log.e("::Unfurling Sagar:::: abcdz ", "action_pair is null for URL: " + expandedURL);
        }

        
        
        ThreadUtils.runOnUiThread(() -> {
            if (action_pair != null) {
                Log.e("::Unfurling Sagar:::: ", "actionURL is: " + action_pair.first);
                callback.onCompletion(action_pair.first, action_pair.second);
            } else if (twitterActionUrl != null && !twitterActionUrl.isEmpty()) {
                Log.e("::Unfurling Sagar:::: ", "actionURL is onlyblink Sagar: " + twitterActionUrl);
                callback.onCompletion(twitterActionUrl, "registered");
            } else {
                Log.e("::Unfurling Sagar:::: ", "actionURL is null");
                callback.onCompletion("", "");
            }
        });
    }

    // Method to get actionUrl by blinkUrl
    public void getActionUrl(String blinkUrl, ActionUrlFetchedCallback callback) {
        // return mActionUrlMap.get(blinkUrl);
        Log.e("::ActionUrl:: kritagya", "getActionUrl:: " + blinkUrl);
        if(!actionListFetched) {
            Log.e("::ActionUrl:: kritagya", "fetching action url list");
            // fetchActionUrlList();
        }
        Thread thread = new Thread(
                () -> {
                    synchronized (mLock) {
                        expandToFinalURL(blinkUrl, callback);
                    }
                });
        thread.start();
    }

    public void fetchActionUrlList() {
        if(actionListFetched) return;
        String url = "https://registry.dial.to/v1/list";
        Thread thread = new Thread(
                () -> {
                    synchronized (mLock) {
                        fetchURLList(url);
                    }
                });
        thread.start();
    }

    // @CalledByNative
    public void fetchURLList(String urlString) {
        Log.e("::ActionUrl:: kritagya", "fetchURLContent:: " + urlString);
        HttpURLConnection connection = null;
        StringBuilder content = new StringBuilder();
        try {
            URL url = new URL(urlString);
            connection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(url,
                    TRAFFIC_ANNOTATION);

            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000); // 5 seconds timeout
            // connection.setReadTimeout(5000); // 5 seconds read timeout
            connection.setRequestProperty("Referrer-Policy", "strict-origin-when-cross-origin");
            int responseCode = connection.getResponseCode();
            if(responseCode == HttpURLConnection.HTTP_OK) {
                InputStream inputStream = connection.getInputStream();
                Log.e("::ActionUrl::", "connection passed");

                // Read the response content fully
                BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line); // Append each line
                }
                Log.e("::ActionUrl:: kritagya", "content is: " + content.toString());
                processFetchedContent(content.toString());
            }
        } catch (Exception e) {
            Log.e("::ActionUrl:: ", "Failed to fetch or parse content: " + e.getMessage());
        } finally {
            if (connection != null) {
                connection.disconnect();
                actionListFetched = true;
            }
        }
    }
    
    // // Interface for JSON download callback
    // public interface JsonDownloadCallback {
    //     void onJsonDownloaded(String jsonContent, int responseCode);
    // }

    // // Method to download JSON using C++ (JNI)
    // private void downloadJsonWithNative(String urlString, JsonDownloadCallback callback) {
    //     Log.e("::Unfurling:::: ", "Downloading JSON with native code: " + urlString);
    //     ActionUrlListFetcherJni.get().downloadJsonFile(urlString, new JsonDownloadCallback() {
    //         @Override
    //         public void onJsonDownloaded(String jsonContent, int responseCode) {
    //             if (responseCode == 200 && !jsonContent.isEmpty()) {
    //                 Log.e("::Unfurling:::: ", "Downloaded JSON successfully");
    //                 callback.onJsonDownloaded(jsonContent, responseCode);
    //             } else {
    //                 Log.e("::Unfurling:::: ", "Failed to download JSON: " + responseCode);
    //                 callback.onJsonDownloaded("", responseCode);
    //             }
    //         }
    //     });
    // }

    // JNI bindings
    // @NativeMethods
    // interface ActionUrlListFetcherJni {
    //     boolean downloadJsonFile(String url, JsonDownloadCallback callback);
    // }

    /**
     * Downloads the action.json file and processes it to find a matching action URL.
     * @param expandedURL The expanded URL to process
     * @return The matching action URL, or empty string if no match is found
     */
    private String processUrlForActionJson(String expandedURL) {
        try {

            // Special handling for dial.to URLs with embedded action URLs
            String actualUrl = expandedURL;
            if (expandedURL.startsWith("https://dial.to/") || expandedURL.startsWith("http://dial.to/")) {
                // Check if there's an embedded URL in the format ?action=solana-action:https://...
                if (expandedURL.contains("?action=solana-action:")) {
                    int startIndex = expandedURL.indexOf("solana-action:") + "solana-action:".length();
                    actualUrl = expandedURL.substring(startIndex);
                    Log.e("::Unfurling:::: ", "Extracted embedded URL from dial.to: " + actualUrl);
                    
                    // If the extracted URL is already an API URL, return it directly
                    if (actualUrl.contains("/api/")) {
                        Log.e("::Unfurling:::: ", "URL is already an API URL, using directly: " + actualUrl);
                        return actualUrl;
                    }
                }
            }
        
            // Parse the URL to extract its components
            URL parsedUrl = new URL(actualUrl);
            String origin = parsedUrl.getProtocol() + "://" + parsedUrl.getHost();
            String path = parsedUrl.getPath();
            String query = parsedUrl.getQuery() != null ? "?" + parsedUrl.getQuery() : "";
            
            // Construct the action.json URL
            String actionJsonUrl = origin + "/actions.json";
            
            Log.e("::Unfurling:::: ", "Fetching action.json from: " + actionJsonUrl);
            
            // Download the action.json using Java (synchronously)
            HttpURLConnection connection = null;
            String jsonContent = "";
            try {
                URL url = new URL(actionJsonUrl);
                connection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(url,
                        TRAFFIC_ANNOTATION);

                connection.setRequestMethod("GET");
                connection.setConnectTimeout(5000);
                connection.setReadTimeout(5000);
                
                int responseCode = connection.getResponseCode();
                if(responseCode == HttpURLConnection.HTTP_OK) {
                    // Read the JSON content
                    InputStream inputStream = connection.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
                    StringBuilder content = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        content.append(line);
                    }
                    jsonContent = content.toString();
                    Log.e("::Unfurling:::: ", "Successfully downloaded action.json");
                } else {
                    Log.e("::Unfurling:::: ", "Failed to download action.json: " + responseCode);
                    return "";
                }
            } catch (Exception e) {
                Log.e("::Unfurling:::: ", "Error downloading action.json: " + e.getMessage());
                return "";
            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
            
            // If we have JSON content, process it
            if (!jsonContent.isEmpty()) {
                return UrlPatternMatcher.findMatchingActionUrl(jsonContent, expandedURL, path, query);
            }
            
            // No match found
            return "";
        } catch (Exception e) {
            Log.e("::Unfurling:::: ", "Error processing URL: " + e.getMessage());
            return "";
        }
    }
}