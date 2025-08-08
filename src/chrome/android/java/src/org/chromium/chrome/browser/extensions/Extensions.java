package org.chromium.chrome.browser.extensions;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Base64;
import android.util.Log;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import java.util.List;
import java.util.ArrayList;
import org.chromium.chrome.browser.ntp.NewTabPageLayout;

import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.wootzapp_search.AiConfig;

public class Extensions {
    private static NewTabPageLayout newTabPageLayout;
    private static final String WOOZAPP_SEARCH_EXTENSION_ID = "lhgoolpdddhhfahbnofaomjhfhfjfhop";

    public static void setNewTabPageLayout(NewTabPageLayout layout) {
        newTabPageLayout = layout;
    }

    public static ArrayList<ExtensionInfo> getExtensionsInfo() {
        String jsonString = ExtensionsJni.get().getExtensionsInfo();
        ArrayList<ExtensionInfo> result = new ArrayList<>();
        try {
            JSONArray array = new JSONArray(jsonString);
            for (int i = 0; i < array.length(); i++) {
                JSONObject obj = array.getJSONObject(i);
                
                // Decode base64 icon
                String iconBase64 = obj.getString("icon_base64");
                Bitmap icon = null;
                if (!iconBase64.isEmpty()) {
                    byte[] iconBytes = Base64.decode(iconBase64, Base64.DEFAULT);
                    icon = BitmapFactory.decodeByteArray(iconBytes, 0, iconBytes.length);
                }
                List<String> features = new ArrayList<>();
                try {
                    JSONArray featuresArray = obj.getJSONArray("features");
                    for (int j = 0; j < featuresArray.length(); j++) {
                        features.add(featuresArray.getString(j));
                    }
                } catch (JSONException e) {
                    Log.e("Extensions", "No features found for " + obj.getString("name"));
                }

                result.add(new ExtensionInfo(
                    obj.getString("id"),
                    obj.getString("name"),
                    obj.getString("description"),
                    obj.getString("popup_url"),
                    obj.getString("widget_url"),
                    icon,
                    features
                ));
            }
        } catch (JSONException e) {
            Log.e("ExtensionInfo", "Error parsing JSON", e);
        }
        return result;
    }

    public static void uninstallExtension(String extensionId) {
        Log.d("Extensions", "Uninstalling extension: " + extensionId);
        ExtensionsJni.get().uninstallExtension(extensionId);
        
        if (extensionId.equals(WOOZAPP_SEARCH_EXTENSION_ID)) {
            AiConfig.clearAiPreferencesForExtension(extensionId);
        }

        notifyExtensionChange();
        
        if (newTabPageLayout != null) {
            Log.d("Extensions", "Reloading new tab page!!");
            newTabPageLayout.reload();
            Log.d("Extensions", "Reloaded new tab page!!");
        }
    }

    /**
     * Notifies ChromeActivity about extension changes
     */
    public static void notifyExtensionChange() {
        try {
            // Get the current ChromeActivity instance
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            if (activity != null) {
                // Call the update method on the main thread
                activity.runOnUiThread(() -> {
                    activity.updateFabVisibility();
                });
            }
        } catch (Exception e) {
            Log.e("Extensions", "Error notifying ChromeActivity: " + e.getMessage());
        }
    }

    @CalledByNative
    public static void notifyExtensionInstalled() {
        Log.e("Extensions", "Extension installed, notifying listeners.");
        // Notify any listeners or update UI as needed
        Extensions.notifyExtensionChange();
    }

    @NativeMethods
    interface Natives {
        String getExtensionsInfo();
        void uninstallExtension(String extensionId);
    }
}