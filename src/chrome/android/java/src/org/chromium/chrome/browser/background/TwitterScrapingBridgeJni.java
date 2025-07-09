package org.chromium.chrome.browser.background;

import org.chromium.base.Log;
import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;
import org.jni_zero.JNINamespace;


@JNINamespace("wootz_scraping")
public class TwitterScrapingBridgeJni {
    private static final String TAG = "TwitterScrapingBridgeJni";
    
    private long mNativeTwitterScrapingBridgeJni;
    private static TwitterScrapingBridgeJni sInstance;
    
    public TwitterScrapingBridgeJni() {
        mNativeTwitterScrapingBridgeJni = TwitterScrapingBridgeJniJni.get().init();
        Log.i(TAG, "TwitterScrapingBridgeJni initialized with native pointer: " + mNativeTwitterScrapingBridgeJni);
    }
    
    public static TwitterScrapingBridgeJni getInstance() {
        if (sInstance == null) {
            sInstance = new TwitterScrapingBridgeJni();
        }
        return sInstance;
    }
    
    public void makeTwitterApiCall() {
        if (mNativeTwitterScrapingBridgeJni == 0) {
            Log.e(TAG, "Native TwitterScrapingBridgeJni is not initialized");
            return;
        }
        
        TwitterScrapingBridgeJniJni.get().makeTwitterApiCall(
            mNativeTwitterScrapingBridgeJni, this);
    }
    
    @CalledByNative
    public void onNativeCallback(String result, boolean success) {
        Log.i(TAG, "Received native callback - Success: " + success + ", Result: " + result);

        if (success) {
            Log.i(TAG, "Twitter API call completed successfully");
        } else {
            Log.e(TAG, "Twitter API call failed: " + result);
        }
    }

    @CalledByNative
    public void onReceiveCredentials(String[] credentials) {
        Log.i(TAG, "Ram -> Received credentials from native:");
        for (String cred : credentials) {
            Log.i(TAG, "Ram -> Credential: " + cred);
        }
        String url = credentials[0];
        String method = credentials[1];
        String headers = credentials[2];
        String body = credentials[3];

        Log.i(TAG, "Ram -> URL: " + url);
        Log.i(TAG, "Ram -> Method: " + method);
        Log.i(TAG, "Ram -> Headers: " + headers);
        Log.i(TAG, "Ram -> Body: " + body);

        makeTwitterApiRequest(url, method, headers, body);
    }

    private void makeTwitterApiRequest(String url, String method, String headersString, String body) {
        new Thread(() -> {
            try {
                java.net.URL apiUrl = new java.net.URL(url);
                java.net.HttpURLConnection conn = (java.net.HttpURLConnection) apiUrl.openConnection();
                conn.setRequestMethod(method);
    
                // Parse headers
                String[] headerLines = headersString.split("\n");
                for (String headerLine : headerLines) {
                    int colonPos = headerLine.indexOf(':');
                    if (colonPos != -1) {
                        String key = headerLine.substring(0, colonPos).trim();
                        String value = headerLine.substring(colonPos + 1).trim();
                        if (!key.isEmpty() && !value.isEmpty()) {
                            conn.setRequestProperty(key, value);
                        }
                    }
                }
    
                // If not GET, send body
                if (!body.isEmpty() && !"GET".equalsIgnoreCase(method)) {
                    conn.setDoOutput(true);
                    try (java.io.OutputStream os = conn.getOutputStream()) {
                        byte[] input = body.getBytes("utf-8");
                        os.write(input, 0, input.length);
                    }
                }
    
                int responseCode = conn.getResponseCode();
                Log.i(TAG, "Ram -> Twitter API HTTP response code: " + responseCode);
                java.io.InputStream is = (responseCode >= 200 && responseCode < 400)
                        ? conn.getInputStream() : conn.getErrorStream();
    
                java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(is));
                StringBuilder response = new StringBuilder();
                String line;
                while ((line = in.readLine()) != null) {
                    response.append(line);
                }
                in.close();
    
                String responseData = response.toString();
                int length = responseData.length();
                String preview = responseData.substring(0, Math.min(1000, length));
                Log.i(TAG, "Ram -> Twitter API response (first 1000 chars): " + preview);
                Log.i(TAG, "Ram -> Twitter API response length: " + length);
                
            } catch (Exception e) {
                Log.e(TAG, "Ram -> Twitter API call failed: " + e.getMessage());
            }
        }).start();
    }
    
    public void destroy() {
        if (mNativeTwitterScrapingBridgeJni != 0) {
            TwitterScrapingBridgeJniJni.get().destroy(mNativeTwitterScrapingBridgeJni, this);
            mNativeTwitterScrapingBridgeJni = 0;
        }
        sInstance = null;
    }

    @NativeMethods
    interface Natives {
        long init();
        void destroy(long nativeTwitterScrapingBridgeJni, TwitterScrapingBridgeJni caller);
        void makeTwitterApiCall(long nativeTwitterScrapingBridgeJni, 
                               TwitterScrapingBridgeJni caller);
    }
}