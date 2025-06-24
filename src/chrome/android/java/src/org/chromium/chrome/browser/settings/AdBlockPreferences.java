// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.settings;

import android.os.Bundle;
import android.util.Log;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;

import org.chromium.content_public.browser.BrowserContextHandle;
import org.chromium.components.browser_ui.site_settings.BaseSiteSettingsFragment;
import org.chromium.components.browser_ui.site_settings.WebsitePreferenceBridge;
import org.chromium.components.content_settings.ContentSettingsType;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import androidx.annotation.VisibleForTesting;
import org.chromium.chrome.R;
import java.util.HashMap;

/**
 * Fragment that allows the user to configure AdBlock related preferences.
 */
public class AdBlockPreferences extends BaseSiteSettingsFragment {
    @VisibleForTesting
    public static final String PREF_ADBLOCK_SWITCH = "adblock_switch";
    private static final String PREF_ADBLOCK_EDIT = "adblock_edit";
    private final HashMap<String, Preference> mRemovedPreferences = new HashMap<>();
    private Preference mAdBlockEdit;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("AdBlock", "AdBlockPreferences onCreate called");
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getActivity().setTitle(R.string.options_adblock_title);
        SettingsUtils.addPreferencesFromResource(this, R.xml.adblock_preferences);
        // configureWootzPreferences();
        BrowserContextHandle browserContextHandle =
                getSiteSettingsDelegate().getBrowserContextHandle();

        ChromeSwitchPreference mAdBlockSwitch =
                (ChromeSwitchPreference) findPreference(PREF_ADBLOCK_SWITCH);
        boolean isAdBlockEnabled = !WebsitePreferenceBridge.isCategoryEnabled(browserContextHandle, ContentSettingsType.ADS);
        Log.i("AdBlock", "AdBlock enabled status: " + isAdBlockEnabled);
        mAdBlockSwitch.setChecked(isAdBlockEnabled);
        mAdBlockSwitch.setOnPreferenceChangeListener((preference, newValue) -> {
            Log.i("AdBlock", "AdBlock switch changed to: " + newValue);
            WebsitePreferenceBridge.setCategoryEnabled(browserContextHandle, ContentSettingsType.ADS, !(boolean) newValue);
            return true;
        });

        mAdBlockEdit = findPreference(PREF_ADBLOCK_EDIT);
        updateCurrentAdBlockUrl();
    }

    private void updateCurrentAdBlockUrl() {
        String currentUrl = ChromeFeatureList.getAdBlockFiltersURL();
        Log.i("AdBlock", "Current AdBlock filters URL: " + currentUrl);
        mAdBlockEdit.setSummary(currentUrl);
        // I have added here to check the adblocking is working or not, but it is not working
        // ChromeFeatureList.setAdBlockFiltersURL(currentUrl);
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.i("AdBlock", "AdBlockPreferences onResume called");
        updateCurrentAdBlockUrl();
    }

    @Override
    public Preference findPreference(CharSequence key) {
        Preference result = super.findPreference(key);
        if (result == null) {
            result = mRemovedPreferences.get((String) key);
            Log.i("AdBlock", "Preference not found in super, checking removed preferences for key: " + key);
        }
        return result;
    }

    private void removePreferenceIfPresent(String key) {
        Preference preference = getPreferenceScreen().findPreference(key);
        if (preference != null) {
            Log.i("AdBlock", "Removing preference with key: " + key);
            getPreferenceScreen().removePreference(preference);
            mRemovedPreferences.put(preference.getKey(), preference);
        }
    }

    private void configureWootzPreferences() {
        Log.i("AdBlock", "Configuring Wootz preferences");
        removePreferenceIfPresent(PREF_ADBLOCK_EDIT);
        removePreferenceIfPresent(PREF_ADBLOCK_SWITCH);
    }
}