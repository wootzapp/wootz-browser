package org.chromium.chrome.browser.extensions;

import android.graphics.Bitmap;

public class ExtensionInfo {
    private String id;
    private String name;
    private String description;
    private String popupUrl;
    private Bitmap iconBitmap;

    // Constructor
    public ExtensionInfo(String id, String name, String description, String popupUrl, Bitmap iconBitmap) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.popupUrl = popupUrl;
        this.iconBitmap = iconBitmap;
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

    public void setPopupUrl(String popupUrl) {
        this.popupUrl = popupUrl;
    }

    public Bitmap getIconBitmap() {
        return iconBitmap;
    }

    public void setIconBitmap(Bitmap iconBitmap) {
        this.iconBitmap = iconBitmap;
    }

    @Override
    public String toString() {
        return "ExtensionInfo:\n" +
            "  Name: '" + name + "'\n" +
            "  Description: '" + description + "'\n" +
            "  Popup URL: '" + popupUrl + "'\n";
    }
}