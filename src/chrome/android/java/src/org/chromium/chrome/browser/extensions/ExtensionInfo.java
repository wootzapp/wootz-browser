package org.chromium.chrome.browser.extensions;

import android.graphics.Bitmap;
import java.util.List;
import java.util.ArrayList;

public class ExtensionInfo {
    private String id;
    private String name;
    private String description;
    private String popupUrl;
    private String widgetUrl;
    private Bitmap iconBitmap;
    private List<String> features;

    // Constructor
    public ExtensionInfo(String id, String name, String description, String popupUrl, String widgetUrl, Bitmap iconBitmap, List<String> features) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.popupUrl = popupUrl;
        this.widgetUrl = widgetUrl;
        this.iconBitmap = iconBitmap;
        this.features = features != null ? features : new ArrayList<>();
    }

    public ExtensionInfo(String id, String name, String description, String popupUrl, String widgetUrl, Bitmap iconBitmap) {
        this(id, name, description, popupUrl, widgetUrl, iconBitmap, new ArrayList<>());
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }
    
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getPopupUrl() {
        return popupUrl;
    }

    public String getWidgetUrl() {
        return widgetUrl;
    }

    public void setPopupUrl(String popupUrl) {
        this.popupUrl = popupUrl;
    }

    public void setWidgetUrl(String widgetUrl) {
        this.widgetUrl = widgetUrl;
    }

    public Bitmap getIconBitmap() {
        return iconBitmap;
    }

    public void setIconBitmap(Bitmap iconBitmap) {
        this.iconBitmap = iconBitmap;
    }

    public List<String> getFeatures() {
        return features;
    }

    public void setFeatures(List<String> features) {
        this.features = features != null ? features : new ArrayList<>();
    }

    @Override
    public String toString() {
        StringBuilder featuresStr = new StringBuilder();
        if (features != null && !features.isEmpty()) {
            featuresStr.append("  Features: [");
            for (int i = 0; i < features.size(); i++) {
                featuresStr.append("'").append(features.get(i)).append("'");
                if (i < features.size() - 1) {
                    featuresStr.append(", ");
                }
            }
            featuresStr.append("]\n");
        } else {
            featuresStr.append("  Features: None\n");
        }
        
        return "ExtensionInfo:\n" +
            "  Name: '" + name + "'\n" +
            "  Description: '" + description + "'\n" +
            "  Popup URL: '" + popupUrl + "'\n" +
            "  Widget URL: '" + widgetUrl + "'\n" +
            featuresStr.toString();
    }
}