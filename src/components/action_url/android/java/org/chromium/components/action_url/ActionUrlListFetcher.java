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
import org.chromium.base.TimeUtils;
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
import java.util.Arrays;
import java.net.URLDecoder;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
// @JNINamespace("action_url")
public class ActionUrlListFetcher{

    private static ActionUrlListFetcher mInstance;
    private static ConcurrentHashMap<String, Pair<String, String>> mActionUrlMap;
    private final ExecutorService threadPool = Executors.newFixedThreadPool(5);
    private volatile boolean isShuttingDown = false;

    // Static cache fields for singleton
    private static final ConcurrentHashMap<String, String> urlExpansionCache = new ConcurrentHashMap<>();
    private static final ConcurrentHashMap<String, String> actionJsonCache = new ConcurrentHashMap<>();
    private static final ConcurrentHashMap<String, Long> actionJsonCacheTimestamps = new ConcurrentHashMap<>();
    private static final long CACHE_TTL_MS = TimeUtils.MILLISECONDS_PER_HOUR;

    private static final Set<String> SUPPORTED_SHORT_DOMAINS = new HashSet<>(Arrays.asList(
        "t.co", 
        "blnk.fun",
        "dial.to"
    ));

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
        mActionUrlMap = new ConcurrentHashMap<>();
        
        // Add shutdown hook
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            isShuttingDown = true;
            shutdownThreadPool();
        }));
    }

    private String formatUrl(String url) {
        if(url.endsWith("/")) {
            return url.substring(0,url.lastIndexOf("/"));
        }
        return url;
    }

    private boolean isShortUrlDomain(String url) {
        try {
            URL parsedUrl = new URL(url);
            String host = parsedUrl.getHost();
            return SUPPORTED_SHORT_DOMAINS.contains(host);
        } catch (Exception e) {
            return false;
        }
    }

    private String parseURL(String url) {
        // Check if there's an embedded URL in the format ?action=solana-action:https://...
        if (url.contains("?action=solana-action:")) {
            int startIndex = url.indexOf("solana-action:") + "solana-action:".length();
            int endIndex = url.indexOf("&", startIndex);
            url = endIndex > 0 ? url.substring(startIndex, endIndex) : url.substring(startIndex);
            Log.e("::Unfurling:::: ", "Extracted embedded URL from dial.to: " + url);
            return url;
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

                if(tags.length() == 0) {
                    tag = "unknown";
                }
                else{
                    tag = tags.getString(0);
                }

                if(blinkUrl != null && !blinkUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(blinkUrl), new Pair<>(actionUrl, tag));
                    // Log.e("::ACTION_URL::", "blinkUrl:: " + blinkUrl + "||| actionUrl:: " + actionUrl);
                }
                if(websiteUrl != null && !websiteUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(websiteUrl), new Pair<>(actionUrl, tag));
                    // Log.e("::ACTION_URL::", "websiteUrl:: " + websiteUrl + "||| actionUrl:: " + actionUrl);
                }

                if(actionUrl != null && !actionUrl.equals("null")) {
                    mActionUrlMap.put(formatUrl(actionUrl), new Pair<>(actionUrl, tag));
                }

            }
        } catch (Exception e) {
            Log.e("ActionUrl::", "Error parsing JSON: " + e.getMessage());
        }
    }

    private String getExpandedURL(String shortUrl) {
        HttpURLConnection httpURLConnection = null;
        String expandedURL = shortUrl;
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
            }
            Log.e("::Unfurling Sagar:::: abcdz ", "expandedURL is: " + expandedURL);

        } catch (Exception e) {
            Log.e("::Unfurling Sagar:::: abcdz ", "Failed to expand blink url: " + e.getMessage());
        } finally {
            if (httpURLConnection != null) {
                httpURLConnection.disconnect();
            }
        }
        return expandedURL;
    }

    // URL expansion cache
    private void expandToFinalURL(String shortUrl, ActionUrlFetchedCallback callback) {

        // if(!isShortUrlDomain(shortUrl)) {
        //     ThreadUtils.runOnUiThread(() -> {
        //         callback.onCompletion("", "");
        //     });
        //     return;
        // }

        // check cache
        String cachedExpandedURL = urlExpansionCache.get(shortUrl);
        if (cachedExpandedURL != null) {
            String actionUrl = processUrlForActionJson(cachedExpandedURL);
            ThreadUtils.runOnUiThread(() -> {
                if(actionUrl != null && !actionUrl.isEmpty()) {
                    callback.onCompletion(actionUrl, "registered");
                } else {
                    callback.onCompletion("", "");
                }
            });
            return;
        }

        String expandedURL = getExpandedURL(shortUrl);
        Pair<String, String> action_pair = mActionUrlMap.get(formatUrl(expandedURL));
        String actionUrl = processUrlForActionJson(expandedURL);
        
        ThreadUtils.runOnUiThread(() -> {
            if (action_pair != null) {
                callback.onCompletion(action_pair.first, action_pair.second);
            } else if (actionUrl != null && !actionUrl.isEmpty()) {
                callback.onCompletion(actionUrl, "registered");
            } else {
                callback.onCompletion("", "");
            }
        });

        // Add to cache after expansion
        urlExpansionCache.put(shortUrl, expandedURL);
    }

    // Method to get actionUrl by blinkUrl
    public void getActionUrl(String blinkUrl, ActionUrlFetchedCallback callback) {
        if (isShuttingDown) {
            Log.w("ActionUrlListFetcher", "Rejecting request during shutdown");
            ThreadUtils.runOnUiThread(() -> {
                callback.onCompletion("", "");
            });
            return;
        }
        
        if(!actionListFetched) {
            // fetchActionUrlList();
        }
        threadPool.submit(() -> {
            expandToFinalURL(blinkUrl, callback);
        });
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
    
    /**
     * Downloads the action.json file and processes it to find a matching action URL.
     * @param expandedURL The expanded URL to process
     * @return The matching action URL, or empty string if no match is found
     */
    private String processUrlForActionJson(String expandedURL) {
        try {
            String actionUrl = "";

            // decode the url to get the actual url utf-8
            String decodedUrl = URLDecoder.decode(expandedURL, "UTF-8");

            if (decodedUrl.contains("dial.to")) {
                actionUrl = parseURL(decodedUrl);
                return actionUrl;
            }
        
            // parsing the url to get the origin, path and query
            URL parsedUrl = new URL(decodedUrl);
            String origin = parsedUrl.getProtocol() + "://" + parsedUrl.getHost();
            String path = parsedUrl.getPath();
            String query = parsedUrl.getQuery() != null ? "?" + parsedUrl.getQuery() : "";
            
            // fetching the action.json file
            String actionJsonUrl = origin + "/actions.json";
            String actionJson = getCachedOrFetchJson(origin, actionJsonUrl);
            
            // If we have JSON content, process it
            if (actionJson != null && !actionJson.isEmpty()) {
                actionUrl = UrlPatternMatcher.findMatchingActionUrl(actionJson, expandedURL, path, query);
            }
            return actionUrl;
        } catch (Exception e) {
            Log.e("::Unfurling:::: ", "Error processing URL: " + e.getMessage());
            return "";
        }
    }

    private String getCachedOrFetchJson(String domain, String url) {
        
        Long timestamp = actionJsonCacheTimestamps.get(domain);
        if (timestamp != null) {
            if(System.currentTimeMillis() - timestamp < CACHE_TTL_MS) {
                String cachedJson = actionJsonCache.get(domain);
                if (cachedJson != null) {
                    return cachedJson;
                }
            } else {
                actionJsonCache.remove(domain);
                actionJsonCacheTimestamps.remove(domain);
            }
        }
        
        // Atomic operation: only one thread will execute this for a given domain
        return actionJsonCache.computeIfAbsent(domain, k -> {
            String json = fetchJsonFromUrl(url);
            if (json != null && !json.isEmpty()) {
                actionJsonCacheTimestamps.put(domain, System.currentTimeMillis());
            }
            return json;
        });
    }

    /**
     * Fetches JSON content from a URL synchronously.
     *
     * @param url The URL to fetch JSON from
     * @return The JSON content as a string, or empty string on failure
     */
    private String fetchJsonFromUrl(String url) {
        HttpURLConnection connection = null;
        try {
            URL urlObj = new URL(url);
            connection = (HttpURLConnection) ChromiumNetworkAdapter.openConnection(
                    urlObj, TRAFFIC_ANNOTATION);

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
                return content.toString();
            } else {
                Log.e("::Unfurling:::: ", "Failed to download JSON: " + responseCode);
                return "";
            }
        } catch (Exception e) {
            Log.e("::Unfurling:::: ", "Error downloading JSON: " + e.getMessage());
            return "";
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    /**
     * Safely shuts down the thread pool, allowing running tasks to complete
     * within a reasonable timeout period.
     */
    private void shutdownThreadPool() {
        Log.i("ActionUrlListFetcher", "Shutting down thread pool");
        
        if (threadPool != null && !threadPool.isShutdown()) {
            try {
                // First attempt a graceful shutdown - no new tasks accepted,
                // but existing tasks will complete
                threadPool.shutdown();
                
                // Wait for existing tasks to complete - using a reasonable timeout
                boolean terminated = threadPool.awaitTermination(3, TimeUnit.SECONDS);
                
                if (!terminated) {
                    // Force immediate shutdown if tasks don't complete in time
                    Log.w("ActionUrlListFetcher", "Thread pool did not terminate in time, forcing shutdown");
                    List<Runnable> pendingTasks = threadPool.shutdownNow();
                    Log.i("ActionUrlListFetcher", "Cancelled " + pendingTasks.size() + " pending tasks");
                }
            } catch (InterruptedException e) {
                // Re-assert the interrupt and force shutdown
                Thread.currentThread().interrupt();
                threadPool.shutdownNow();
                Log.w("ActionUrlListFetcher", "Thread pool shutdown interrupted, forcing immediate shutdown");
            } catch (Exception e) {
                // Catch any other exceptions to ensure shutdown completes
                Log.e("ActionUrlListFetcher", "Error during thread pool shutdown: " + e.getMessage());
                threadPool.shutdownNow();
            }
        }
    }
    
    /**
     * Explicitly shutdown the instance, releasing resources.
     * Call this when you know the ActionUrlListFetcher is no longer needed.
     */
    public void shutdown() {
        shutdownThreadPool();
    }
    
    // Add a cleanup method for singleton pattern
    public static void destroyInstance() {
        if (mInstance != null) {
            mInstance.shutdown();
            mInstance = null;
        }
    }
}