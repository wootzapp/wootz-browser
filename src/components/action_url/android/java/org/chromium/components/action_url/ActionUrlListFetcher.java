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

            // Populate the HashMap with blinkUrl as key and actionUrl as value
            for (int i = 0; i < resultsArray.length(); i++) {
                JSONObject resultObject = resultsArray.getJSONObject(i);
                String blinkUrl = resultObject.getString("blinkUrl");
                String actionUrl = resultObject.getString("actionUrl");
                String websiteUrl = resultObject.getString("websiteUrl");
                JSONArray tags = resultObject.getJSONArray("tags");
                String tag;
                if(tags.length() == 0) {
                    tag = "unknown";
                }
                else{
                    tag = tags.getString(0);
                }

                if(!blinkUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(blinkUrl), new Pair<>(actionUrl, tag));
                    Log.i("::ACTION_URL::", "blinkUrl:: " + blinkUrl + "||| actionUrl:: " + actionUrl);
                }
                if(!websiteUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(websiteUrl), new Pair<>(actionUrl, tag));
                    Log.i("::ACTION_URL::", "websiteUrl:: " + websiteUrl + "||| actionUrl:: " + actionUrl);
                }

                mActionUrlMap.put(formatUrl(actionUrl), new Pair<>(actionUrl, tag));

            }
        } catch (Exception e) {
            Log.i("ActionUrl::", "Error parsing JSON: " + e.getMessage());
        }
    }

    private void expandToFinalURL(String shortUrl,  ActionUrlFetchedCallback callback) {
        HttpURLConnection httpURLConnection = null;
        String expandedURL = shortUrl;
        Log.i("::Unfurling :::: ", "expandUrl url is: "+shortUrl);
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
            Log.i("::Unfurling :::: ", "Response code is: " + responseCode);
            if (responseCode == HttpURLConnection.HTTP_MOVED_PERM || 
                responseCode == HttpURLConnection.HTTP_MOVED_TEMP || 
                responseCode == HttpURLConnection.HTTP_SEE_OTHER) {
                // Get the "Location" header to find the redirect target
                expandedURL = httpURLConnection.getHeaderField("Location");  
            } else {
                // If there's no redirection, return the original URL
                expandedURL = shortUrl;
            }

        } catch (Exception e) {
            Log.i("::Unfurling :::: ", "Failed to expand blink url: " + e.getMessage());
        } finally {
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
            }
        }

        Log.i("::Unfurling :::: ", "expandedURL is: " + expandedURL);    
        Pair<String, String> action_pair = mActionUrlMap.get(formatUrl(expandedURL));
        
        ThreadUtils.runOnUiThread(() -> {
            if(action_pair != null) {
                Log.i("::Unfurling :::: ", "actionURL is: " + action_pair.first);
                callback.onCompletion(action_pair.first, action_pair.second);
            }
            else{
                Log.i("::Unfurling :::: ", "actionURL is##: null"); 
                callback.onCompletion("", "");
            }
        });

    }

    // Method to get actionUrl by blinkUrl
    public void getActionUrl(String blinkUrl, ActionUrlFetchedCallback callback) {
        // return mActionUrlMap.get(blinkUrl);
        if(!actionListFetched) {
            fetchActionUrlList();
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
        Log.i("::ActionUrl::", "fetchURLContent:: " + urlString);
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
                Log.i("::ActionUrl::", "connection passed");

                // Read the response content fully
                BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line); // Append each line
                }

                processFetchedContent(content.toString());
            }
        } catch (Exception e) {
            Log.i("::ActionUrl:: ", "Failed to fetch or parse content: " + e.getMessage());
        } finally {
            if (connection != null) {
                connection.disconnect();
                actionListFetched = true;
            }
        }
    }
    
}