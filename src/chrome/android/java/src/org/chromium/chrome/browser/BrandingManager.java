// BrandingManager.java
package org.chromium.chrome.browser;

import android.content.SharedPreferences;
import android.text.TextUtils;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.chromium.base.ContextUtils;
import org.chromium.base.task.AsyncTask;
import org.chromium.chrome.R;
import org.json.JSONArray;
import org.json.JSONObject;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Scanner;

public class BrandingManager {
    private static final String TAG = "BrandingManager";
    
    // GitHub URL - using your actual repository
    private static final String GITHUB_URL = "https://raw.githubusercontent.com/wootzapp/ext-store/main/extensions.json";
    
    // Simple keys
    private static final String KEY_APP_NAME = "app_name";
    private static final String KEY_UTM = "utm_source";
    private static final String KEY_ICON_URL = "icon_url";
    
    /**
     * One simple method - fetch from GitHub and save.
     * Call this when you get a UTM source.
     */
    public static void fetchAndSave(String utmSource) {
        Log.e(TAG, "fetchAndSave: " + utmSource);
        if (TextUtils.isEmpty(utmSource)) return;
        
        new AsyncTask<Void>() {
            @Override
            protected Void doInBackground() {
                Log.e(TAG, "doInBackground: " + utmSource);
                try {
                    // Get JSON from GitHub
                    URL url = new URL(GITHUB_URL);
                    HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                    InputStream input = conn.getInputStream();
                    Scanner scanner = new Scanner(input).useDelimiter("\\A");
                    String json = scanner.hasNext() ? scanner.next() : "";
                    conn.disconnect();
                    Log.e(TAG, "json: " + json);
                    // Parse and find matching UTM
                    JSONArray brands = new JSONObject(json).getJSONArray("extensions");
                    for (int i = 0; i < brands.length(); i++) {
                        JSONObject brand = brands.getJSONObject(i);
                        if (utmSource.equals(brand.optString("campaign"))) {
                            // Save to preferences
                            String appName = brand.optString("name", "Browser");
                            String iconUrl = brand.optString("icon_url", "");
                            ContextUtils.getAppSharedPreferences().edit()
                                .putString(KEY_APP_NAME, appName)
                                .putString(KEY_UTM, utmSource)
                                .putString(KEY_ICON_URL, iconUrl)
                                .apply();
                            Log.e(TAG, "Saved: " + appName);
                            Log.e(TAG, "Saved: " + utmSource);
                            Log.e(TAG, "Saved icon URL: " + iconUrl);
                            break;
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error fetching branding", e);
                }
                return null;
            }
            
            @Override
            protected void onPostExecute(Void result) {
                // This method is required but we don't need to do anything here
                // since we're not updating UI from this task
            }
        }.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }
    
    /**
     * Get current app name.
     */
    @NonNull
    public static String getAppName() {
        return ContextUtils.getAppSharedPreferences().getString(KEY_APP_NAME, "Browser");
    }
    
    /**
     * Set app name in TextView.
     */
    public static void setTitle(@Nullable TextView textView) {
        if (textView != null) textView.setText(getAppName());
    }
    
    /**
     * Set settings title in TextView.
     */
    public static void setSettingsTitle(@Nullable TextView textView) {
        if (textView != null) textView.setText(getAppName() + " Settings");
    }
    
    /**
     * Set incognito title in TextView.
     */
    public static void setIncognitoTitle(@Nullable TextView textView) {
        if (textView != null) textView.setText(getAppName() + " Incognito");
    }
    
    /**
     * Set default icon in ImageView.
     */
    public static void setIcon(@Nullable ImageView imageView) {
        if (imageView != null) imageView.setImageResource(R.drawable.ic_chrome);
    }
}