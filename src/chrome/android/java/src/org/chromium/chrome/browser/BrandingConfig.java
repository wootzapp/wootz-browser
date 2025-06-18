package org.chromium.chrome.browser;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONObject;

public class BrandingConfig {
    private String utmSource;
    private String appName;
    private String iconUrl;
    private String packageName;
    private String primaryColor;
    private String accentColor;
    
    public BrandingConfig() {}
    
    public static BrandingConfig fromJson(JSONObject json) {
        BrandingConfig config = new BrandingConfig();
        config.utmSource = json.optString("utmSource", "");
        config.appName = json.optString("appName", "");
        config.iconUrl = json.optString("iconUrl", "");
        config.packageName = json.optString("packageName", "");
        config.primaryColor = json.optString("primaryColor", "");
        config.accentColor = json.optString("accentColor", "");
        return config;
    }

    public JSONObject toJson() {
        JSONObject json = new JSONObject();
        try {
            json.put("utmSource", utmSource);
            json.put("appName", appName);
            json.put("iconUrl", iconUrl);
            json.put("packageName", packageName);
            json.put("primaryColor", primaryColor);
            json.put("accentColor", accentColor);
        } catch (Exception e) {
            // Handle JSON exception
        }
        return json;
    }

    // Getters and setters
    @NonNull
    public String getUtmSource() { return utmSource; }
    public void setUtmSource(String utmSource) { this.utmSource = utmSource; }

    @NonNull
    public String getAppName() { return appName; }
    public void setAppName(String appName) { this.appName = appName; }

    @Nullable
    public String getIconUrl() { return iconUrl; }
    public void setIconUrl(String iconUrl) { this.iconUrl = iconUrl; }

    @NonNull
    public String getPackageName() { return packageName; }
    public void setPackageName(String packageName) { this.packageName = packageName; }

    @Nullable
    public String getPrimaryColor() { return primaryColor; }
    public void setPrimaryColor(String primaryColor) { this.primaryColor = primaryColor; }

    @Nullable
    public String getAccentColor() { return accentColor; }
    public void setAccentColor(String accentColor) { this.accentColor = accentColor; }
}