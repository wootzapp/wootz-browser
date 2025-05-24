package org.chromium.chrome.browser.tracing.settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreference;
import androidx.preference.SwitchPreferenceCompat;

import org.chromium.chrome.R;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogProperties;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.base.ContextUtils;

import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;
import org.jni_zero.JNINamespace;


@JNINamespace("chrome::android")
public class ExtensionDeveloperModeSettings extends PreferenceFragmentCompat
        implements SharedPreferences.OnSharedPreferenceChangeListener {
    
    private static final String PREF_EXTENSION_DEVELOPER_MODE = "extension_developer_mode_toggle";
    
    private androidx.preference.SwitchPreferenceCompat mDeveloperModeToggle;
    private SharedPreferences mSharedPreferences;
    
    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getActivity().setTitle(R.string.menu_developer_mode);
        
        // Load the preferences from XML
        setPreferencesFromResource(R.xml.extension_developer_mode_preferences, rootKey);
        
        mSharedPreferences = PreferenceManager.getDefaultSharedPreferences(getActivity());
        mSharedPreferences.registerOnSharedPreferenceChangeListener(this);
        Log.e("ExtensionDeveloperModeSettings", "onCreatePreferences");
        // Initialize the toggle preference
        mDeveloperModeToggle = (androidx.preference.SwitchPreferenceCompat) findPreference(PREF_EXTENSION_DEVELOPER_MODE);
        if (mDeveloperModeToggle != null) {
            updateDeveloperModeToggleState();
            // No dialog, just update the preference directly
            mDeveloperModeToggle.setOnPreferenceChangeListener((preference, newValue) -> {
                Log.e("ExtensionDeveloperModeSettings", "onPreferenceChangeListener: " + newValue);
                boolean enabled = (Boolean) newValue;
                return true; // Allow the preference to be changed directly
            });
        }
    }
    
    @Override
    public void onDestroy() {
        mSharedPreferences.unregisterOnSharedPreferenceChangeListener(this);
        super.onDestroy();
    }
    
    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        Log.e("ExtensionDeveloperModeSettings", "onSharedPreferenceChanged: " + key);
        if (PREF_EXTENSION_DEVELOPER_MODE.equals(key)) {
            updateDeveloperModeToggleState();
        }
    }
    
    private void updateDeveloperModeToggleState() {
        boolean enabled = mSharedPreferences.getBoolean(PREF_EXTENSION_DEVELOPER_MODE, false);
        mDeveloperModeToggle.setChecked(enabled);
        
        // Update the summary based on the current state
        if (enabled) {
            mDeveloperModeToggle.setSummary(R.string.developer_mode_enabled_description);
            isDeveloperModeEnabled(true);
        } else {
            mDeveloperModeToggle.setSummary(R.string.developer_mode_disabled_description);
            isDeveloperModeEnabled(false);
        }
    }
    
    public static ExtensionDeveloperModeSettings newInstance() {
        Log.e("ExtensionDeveloperModeSettings", "newInstance");
        return new ExtensionDeveloperModeSettings();
    }
    
    public static void isDeveloperModeEnabled(boolean isDeveloperModeEnabled) {
        Log.e("ExtensionDeveloperModeSettings", "isDeveloperModeEnabled: " + isDeveloperModeEnabled);
        ExtensionDeveloperModeSettingsJni.get().isDeveloperModeEnabled(isDeveloperModeEnabled);
    }

    @NativeMethods
    interface Natives {
        void isDeveloperModeEnabled(boolean isDeveloperModeEnabled);
    }

}
