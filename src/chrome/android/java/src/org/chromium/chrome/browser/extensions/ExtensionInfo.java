package org.chromium.chrome.browser.extensions;

import android.graphics.Bitmap;

public class ExtensionInfo {

    private String name;
    private String description;
    private String popupUrl;
    private Bitmap iconBitmap;

    // Constructor
    public ExtensionInfo(String name, String description, String popupUrl, Bitmap iconBitmap) {
        this.name = name;
        this.description = description;
        this.popupUrl = popupUrl;
        this.iconBitmap = iconBitmap;
    }

    // Getters and setters
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