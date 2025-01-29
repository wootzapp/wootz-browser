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

public class Extensions {
    private static NewTabPageLayout newTabPageLayout;

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
                result.add(new ExtensionInfo(
                    obj.getString("id"),
                    obj.getString("name"),
                    obj.getString("description"),
                    obj.getString("popup_url"),
                    obj.getString("widget_url"),
                    icon
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
        if (newTabPageLayout != null) {
            Log.d("Extensions", "Reloading new tab page!!");
            newTabPageLayout.reload();
            Log.d("Extensions", "Reloaded new tab page!!");
        }
    }

    @NativeMethods
    interface Natives {
        String getExtensionsInfo();
        void uninstallExtension(String extensionId);
    }
}
