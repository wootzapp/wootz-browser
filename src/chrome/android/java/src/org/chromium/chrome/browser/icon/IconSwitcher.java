package org.chromium.chrome.browser.icon;

import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageManager;
import android.util.Log;

import org.chromium.base.ContextUtils;

/**
 * Handles switching app icons based on configuration.
 */
public class IconSwitcher {
    private static final String TAG = "IconSwitcher";
    
    // Icon types
    public static final String ICON_DEFAULT = "default";
    public static final String ICON_CAMP = "camp";
    public static final String ICON_ARTIFACT = "artifact";
    public static final String ICON_SAPIEN = "sapien";
    public static final String ICON_BLOCKMESH = "blockmesh";
    public static final String ICON_ECLIPSE = "eclipse";
    
    // Component names for the activity aliases
    private static final String DEFAULT_ACTIVITY = "com.google.android.apps.chrome.Main";
    private static final String CAMP_ACTIVITY = "org.chromium.chrome.browser.CampIcon";
    private static final String ARTIFACT_ACTIVITY = "org.chromium.chrome.browser.ArtifactIcon";
    private static final String SAPIEN_ACTIVITY = "org.chromium.chrome.browser.SapienIcon";
    private static final String BLOCKMESH_ACTIVITY = "org.chromium.chrome.browser.BlockMeshIcon";
    private static final String ECLIPSE_ACTIVITY = "org.chromium.chrome.browser.EclipseIcon";
    private static String logo;
    /**
     * Sets the app icon to the specified type.
     * @param iconType The type of icon to set
     */
    public static void setIcon(String utmSource) {
        Log.d("IconSwitcher", "setIcon");
        Context context = ContextUtils.getApplicationContext();
        PackageManager pm = context.getPackageManager();
        String packageName = context.getPackageName();
        
        // Disable all icons first
        disableComponent(pm, packageName, DEFAULT_ACTIVITY);
        disableComponent(pm, packageName, CAMP_ACTIVITY);
        disableComponent(pm, packageName, ARTIFACT_ACTIVITY);
        disableComponent(pm, packageName, SAPIEN_ACTIVITY);
        disableComponent(pm, packageName, BLOCKMESH_ACTIVITY);
        disableComponent(pm, packageName, ECLIPSE_ACTIVITY);
        
        // Enable the selected icon
        switch (utmSource) {
            case ICON_CAMP:
                Log.d("IconSwitcher", "ICON_CAMP");
                enableComponent(pm, packageName, CAMP_ACTIVITY);
                logo = CAMP_ACTIVITY;
                break;
            case ICON_ARTIFACT:
                enableComponent(pm, packageName, ARTIFACT_ACTIVITY);
                logo = ARTIFACT_ACTIVITY;
                break;
            case ICON_SAPIEN:
                enableComponent(pm, packageName, SAPIEN_ACTIVITY);
                logo = SAPIEN_ACTIVITY;
                break;
            case ICON_BLOCKMESH:
                enableComponent(pm, packageName, BLOCKMESH_ACTIVITY);
                logo = BLOCKMESH_ACTIVITY;
                break;
            case ICON_ECLIPSE:
                enableComponent(pm, packageName, ECLIPSE_ACTIVITY);
                logo = ECLIPSE_ACTIVITY;
                break;
            case ICON_DEFAULT:
            default:
                Log.d("IconSwitcher", "ICON_DEFAULT");
                enableComponent(pm, packageName, DEFAULT_ACTIVITY);
                logo = DEFAULT_ACTIVITY;
                break;
        }
        
        Log.d(TAG, "App icon switched to: " + utmSource);
    }
    
    private static void enableComponent(PackageManager pm, String packageName, String componentName) {
        Log.d("IconSwitcher", "enableComponent");
        pm.setComponentEnabledSetting(
                new ComponentName(packageName, componentName),
                PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                PackageManager.DONT_KILL_APP);
    }
    
    private static void disableComponent(PackageManager pm, String packageName, String componentName) {
        Log.d("IconSwitcher", "disableComponent");
        pm.setComponentEnabledSetting(
                new ComponentName(packageName, componentName),
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);
    }
    public static String getLogo() {
        return logo;
    }
}