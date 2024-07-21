package org.chromium.chrome.browser.toolbar.settings;

import android.os.Bundle;
import android.app.Activity;
import androidx.appcompat.app.AlertDialog;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import android.content.SharedPreferences;

import android.content.DialogInterface;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ApplicationLifetime;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
// import org.chromium.chrome.browser.WootzAppRelaunchUtils;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.chrome.browser.toolbar.settings.ToolbarSettings;

public class ToolbarSettings extends PreferenceFragmentCompat implements Preference.OnPreferenceChangeListener {
    private static final String PREF_ENABLE_BOTTOM_TOOLBAR = "enable_bottom_toolbar";
    
    public ToolbarSettings() {
        setHasOptionsMenu(true);		  
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getActivity().setTitle(R.string.preferences_toolbar);
        SettingsUtils.addPreferencesFromResource(this, R.xml.toolbar_preferences);

    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String key = preference.getKey();
        boolean shouldRelaunch = false;

        if (PREF_ENABLE_BOTTOM_TOOLBAR.equals(key)) {
            shouldRelaunch = true;
        }
        if (shouldRelaunch) {
            // WootzAppRelaunchUtils.askForRelaunch(getActivity());
        }
        
        return true;
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        ChromeSwitchPreference enableBottomToolbar = (ChromeSwitchPreference) findPreference(PREF_ENABLE_BOTTOM_TOOLBAR);
        if (enableBottomToolbar != null) {
            enableBottomToolbar.setOnPreferenceChangeListener(this);
        }
    }
}