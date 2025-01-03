// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.language;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Build;
import android.preference.PreferenceManager;
import android.text.TextUtils;

import androidx.annotation.ChecksSdkIntAtLeast;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import org.chromium.base.BuildInfo;
import org.chromium.base.BundleUtils;
import org.chromium.base.LocaleUtils;
import org.chromium.chrome.browser.preferences.ChromePreferenceKeys;
import org.chromium.chrome.browser.preferences.ChromeSharedPreferences;
import org.chromium.components.language.LocaleManagerDelegate;
import org.chromium.components.language.LocaleManagerDelegateImpl;
import org.chromium.ui.base.ResourceBundle;

import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;

import org.chromium.base.Log;
import org.chromium.base.ContextUtils;

/**
 * Provides utility functions to assist with overriding the application language.
 * This class manages the AppLanguagePref.
 */
public class AppLocaleUtils {
    private static final String TAG = "AppLocaleUtils";
    private AppLocaleUtils() {}

    // Value of AppLocale preference when the system language is used.
    public static final String APP_LOCALE_USE_SYSTEM_LANGUAGE = null;

    /**
     * Return true if languageName is the same as the current application override
     * language stored preference.
     * @return boolean
     */
    public static boolean isAppLanguagePref(String languageName) {
        return TextUtils.equals(getAppLanguagePref(), languageName);
    }

    /**
     * The |ApplocaleUtils.APP_LOCALE_USE_SYSTEM_LANGUAGE| constant acts as a signal that no app
     * override language is set and when this is the case the app UI language tracks the device
     * language.
     * @param overrideLanguage String to compare to the default system language value.
     * @return Whether or not |overrideLanguage| is the default system language.
     */
    public static boolean isFollowSystemLanguage(String overrideLanguage) {
        return TextUtils.equals(overrideLanguage, APP_LOCALE_USE_SYSTEM_LANGUAGE);
    }

    /**
     * Get the value of application language shared preference or null if there is none. On T+ this
     * method will use the {@link LocaleManager} service to get the App language.
     * @return String BCP-47 language tag (e.g. en-US).
     */
    public static String getAppLanguagePref() {
        if (shouldUseSystemManagedLocale()) {
            return getSystemManagedAppLanguage();
        }
        return ChromeSharedPreferences.getInstance()
                .readString(
                        ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE,
                        APP_LOCALE_USE_SYSTEM_LANGUAGE);
    }

    /**
     * Get the value of application language shared preference or null if there is none.
     * Used during {@link ChromeApplication#attachBaseContext} before
     * {@link ChromeSharedPreferences} is created.
     * @param base Context to use for getting the shared preference.
     * @return String BCP-47 language tag (e.g. en-US).
     */
    @SuppressWarnings("DefaultSharedPreferencesCheck")
    static String getAppLanguagePrefStartUp(Context base) {
        return PreferenceManager.getDefaultSharedPreferences(base)
                .getString(
                        ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE,
                        APP_LOCALE_USE_SYSTEM_LANGUAGE);
    }

    /**
     * Get the value of system App language using {@link LocaleManager}, Android ensures this
     * language is always supported by Chrome. If no override language is set
     * |APP_LOCALE_USE_SYSTEM_LANGUAGE| is returned. Only used on Android T (API level 33).
     * TODO(crbug.com/40228013) Move to Android T.
     */
    @RequiresApi(Build.VERSION_CODES.S)
    static @Nullable String getSystemManagedAppLanguage() {
        Locale locale = getAppLocaleManagerDelegate().getApplicationLocale();
        if (locale == null) {
            return APP_LOCALE_USE_SYSTEM_LANGUAGE;
        }
        return locale.toLanguageTag();
    }

    /**
     * Gets the first original system locale from {@link LocaleManager}. This is the language that
     * Chrome would use if there was no override set. If there are no possible UI languages en-US is
     * returned since that is the default UI language in that case. Only used on Android T (API
     * level 33). TODO(crbug.com/40228013) Move to Android T.
     *
     * @return The UI language of the system.
     */
    @RequiresApi(Build.VERSION_CODES.S)
    static Locale getSystemManagedOriginalLocale() {
        List<Locale> locales = getAppLocaleManagerDelegate().getSystemLocales();
        for (Locale locale : locales) {
            if (isSupportedUiLanguage(locale.toLanguageTag())) {
                return locale;
            }
        }
        return Locale.forLanguageTag("en-US");
    }

    /**
     * Download the language split. If successful set the application language shared preference.
     * If set to null the system language will be used.
     * @param languageName String BCP-47 code of language to download.
     */
    public static void setAppLanguagePref(String languageName) {
        setAppLanguagePref(languageName, success -> {});
    }

    /**
     * Download the language split using the provided listener for callbacks. If successful set the
     * application language shared preference. If called from an APK build where no bundle needs to
     * be downloaded the listener's on complete function is immediately called. If languageName is
     * null the system language will be used.
     * @param languageName String BCP-47 code of language to download.
     * @param listener LanguageSplitInstaller.InstallListener to use for callbacks.
     */
    public static void setAppLanguagePref(
            String languageName, LanguageSplitInstaller.InstallListener listener) {
        Log.i(TAG, "Setting app language preference: " + languageName);
        Log.i(TAG, "Current app language: " + getAppLanguagePref());
        Log.i(TAG, "Is system managed locale: " + shouldUseSystemManagedLocale());
        Log.i(TAG, "Android SDK version: " + Build.VERSION.SDK_INT);
        Log.i(TAG, "Is bundle build: " + BundleUtils.isBundle());
        
        // Validate language support first
        if (!isSupportedUiLanguage(languageName)) {
            Log.e(TAG, "Unsupported language: " + languageName);
            listener.onComplete(false);
            return;
        }

        // Create a wrapped listener that handles both installation and configuration
        LanguageSplitInstaller.InstallListener wrappedListener =
                (success) -> {
                    if (success) {
                        try {
                            Context context = ContextUtils.getApplicationContext();
                            
                            // First verify if language is supported on this device
                            if (!isLanguageSupportedOnDevice(languageName)) {
                                Log.e(TAG, "Language not supported on this device: " + languageName);
                                success = false;
                                listener.onComplete(false);
                                return;
                            }
                            
                            // First update shared preferences
                            ChromeSharedPreferences.getInstance()
                                    .writeString(
                                            ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE,
                                            languageName);
                            
                            // Handle system-managed locale based on device manufacturer
                            if (shouldUseSystemManagedLocale()) {
                                try {
                                    String manufacturer = Build.MANUFACTURER.toLowerCase();
                                    Log.i(TAG, "Device manufacturer: " + manufacturer);
                                    
                                    if (manufacturer.contains("samsung") || manufacturer.contains("oneplus")) {
                                        // For Samsung and OnePlus, we need to ensure configuration is updated first
                                        updateAppConfiguration(context, languageName);
                                        setSystemManagedAppLanguage(languageName);
                                    } else {
                                        // For other devices (like Pixel), use standard flow
                                        setSystemManagedAppLanguage(languageName);
                                        updateAppConfiguration(context, languageName);
                                    }
                                    
                                    Log.i(TAG, "After setting system managed locale, app language is: " + getAppLanguagePref());
                                } catch (Exception e) {
                                    Log.e(TAG, "Error setting system managed locale", e);
                                    success = false;
                                }
                            } else {
                                // For non-system managed locale, just update configuration
                                updateAppConfiguration(context, languageName);
                            }
                            
                            // Verify language resources after configuration update
                            if (!verifyLanguageResources(languageName)) {
                                Log.e(TAG, "Language resources not available after configuration update: " + languageName);
                                success = false;
                            } else {
                                // Notify GlobalAppLocaleController only if verification succeeds
                                try {
                                    GlobalAppLocaleController.getInstance().maybeSetupLocaleManager();
                                    Log.i(TAG, "Notified GlobalAppLocaleController of locale change");
                                } catch (Exception e) {
                                    Log.e(TAG, "Error notifying locale change", e);
                                }
                            }
                            
                            Log.i(TAG, "Language change process completed with success: " + success);
                        } catch (Exception e) {
                            Log.e(TAG, "Failed to set language preference", e);
                            e.printStackTrace();
                            success = false;
                        }
                    } else {
                        Log.e(TAG, "Failed to install language: " + languageName);
                    }
                    listener.onComplete(success);
                };

        try {
            if (!BundleUtils.isBundle()) {
                Log.i(TAG, "Non-bundle build, completing immediately");
                wrappedListener.onComplete(true);
            } else if (isFollowSystemLanguage(languageName)) {
                Log.i(TAG, "Using system language, completing immediately");
                wrappedListener.onComplete(true);
            } else {
                Log.i(TAG, "Installing language split for: " + languageName);
                // Add retry mechanism for language installation
                installLanguageWithRetry(languageName, wrappedListener, 3);
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception during language installation", e);
            e.printStackTrace();
            wrappedListener.onComplete(false);
        }
    }

    private static void installLanguageWithRetry(String languageName, 
            LanguageSplitInstaller.InstallListener listener, int maxRetries) {
        installLanguageWithRetryInternal(languageName, listener, maxRetries, 0);
    }

    private static void installLanguageWithRetryInternal(String languageName,
            LanguageSplitInstaller.InstallListener listener, int maxRetries, int currentRetry) {
        if (currentRetry >= maxRetries) {
            Log.e(TAG, "Failed to install language after " + maxRetries + " retries: " + languageName);
            listener.onComplete(false);
            return;
        }

        LanguageSplitInstaller.InstallListener retryListener = (success) -> {
            if (success) {
                listener.onComplete(true);
            } else {
                Log.w(TAG, "Retry " + (currentRetry + 1) + " failed for language: " + languageName);
                // Wait briefly before retrying
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    // Ignore
                }
                installLanguageWithRetryInternal(languageName, listener, maxRetries, currentRetry + 1);
            }
        };

        LanguageSplitInstaller.getInstance().installLanguage(languageName, retryListener);
    }

    private static boolean verifyLanguageResources(String languageName) {
        try {
            Context context = ContextUtils.getApplicationContext();
            if (context == null) return false;

            // Try to load a basic string resource with the new locale
            Configuration config = new Configuration(context.getResources().getConfiguration());
            config.setLocale(Locale.forLanguageTag(languageName));
            Context localizedContext = context.createConfigurationContext(config);
            
            // Verify that resources can be loaded
            try {
                localizedContext.getResources().getString(android.R.string.ok);
                Log.i(TAG, "Successfully verified language resources for: " + languageName);
                return true;
            } catch (Exception e) {
                Log.e(TAG, "Failed to load resources for language: " + languageName, e);
                return false;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error verifying language resources", e);
            return false;
        }
    }

    /**
     * Sets the system managed locale for Android 13+ devices.
     * @param languageName The BCP-47 language tag to set
     */
    @RequiresApi(Build.VERSION_CODES.S)
    public static void setSystemManagedAppLanguage(String languageName) {
        try {
            LocaleManagerDelegate delegate = getAppLocaleManagerDelegate();
            if (languageName == null || isFollowSystemLanguage(languageName)) {
                Log.i(TAG, "Clearing system managed locale to use system language");
                delegate.setApplicationLocale(null);
            } else {
                Log.i(TAG, "Setting system managed locale to: " + languageName);
                delegate.setApplicationLocale(languageName);
            }
            
            // Also update configuration directly to ensure immediate effect
            Context context = ContextUtils.getApplicationContext();
            if (context != null) {
                try {
                    Configuration config = new Configuration(context.getResources().getConfiguration());
                    Locale newLocale = Locale.forLanguageTag(languageName != null ? languageName : 
                            Locale.getDefault().toLanguageTag());
                    config.setLocale(newLocale);
                    Locale.setDefault(newLocale);
                    context.getResources().updateConfiguration(config, context.getResources().getDisplayMetrics());
                    Log.i(TAG, "Updated configuration with locale: " + newLocale);
                } catch (Exception e) {
                    Log.e(TAG, "Error updating configuration", e);
                }
            }
            
            // Verify the change
            Locale currentLocale = delegate.getApplicationLocale();
            Log.i(TAG, "System managed locale after change: " + 
                (currentLocale != null ? currentLocale.toLanguageTag() : "null"));
        } catch (Exception e) {
            Log.e(TAG, "Error setting system managed locale", e);
            throw e; // Rethrow to handle in caller
        }
    }

    /**
     * Get the LocaleManagerDelegate for {@link LocaleManager}. Only used on Android T+ (API level
     * 33). TODO(crbug.com/40228013) Move to Android T.
     */
    @RequiresApi(Build.VERSION_CODES.S)
    static LocaleManagerDelegate getAppLocaleManagerDelegate() {
        return new LocaleManagerDelegateImpl();
    }

    /**
     * Migrate the App override language from Chrome SharedPreferences to the {@link LocaleManager}
     * service if needed. A migration is only attempted once on Android T and done if there is a
     * Chrome SharedPreferences override language but no system App override language.
     * TODO(crbug.com/40228013) Move to Android T. TODO(crbug.com/40846627) Remove migration after
     * Oct 2023.
     */
    @RequiresApi(Build.VERSION_CODES.S)
    public static void maybeMigrateOverrideLanguage() {
        // Don't migrate if there is no SharedPreference for the override language.
        // Since null is saved in the SharedPreference if following the system language, a custom
        // token is used for when the preference is not present.
        String unsetToken = "__UNSET__";
        String sharedPrefAppLanguage =
                ChromeSharedPreferences.getInstance()
                        .readString(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE, unsetToken);
        if (TextUtils.equals(sharedPrefAppLanguage, unsetToken)) return;

        // Removed the old shared preference so a migration will not occur again.
        removeSharedPrefAppLanguage();

        // Don't migrate if the old override language was set to follow the system.
        if (isFollowSystemLanguage(sharedPrefAppLanguage)) return;

        // Don't migrate if the Android system already has an App override language. This means that
        // before the migration occurred a user set an App over language in the Android Settings.
        if (!TextUtils.isEmpty(getAppLanguagePref())) return;

        // Set the existing override language as the system App override language.
        setSystemManagedAppLanguage(sharedPrefAppLanguage);
    }

    private static void removeSharedPrefAppLanguage() {
        ChromeSharedPreferences.getInstance()
                .removeKey(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE);
    }

    /**
     * The LocaleManager API is only available on Android T. While using pre-release SDKs it is not
     * possible to use Build.VERSION_CODES.T. This method uses {@link BuildInfo.isAtLeastT} to check
     * that the current SDK is T (API level 33). TODO(crbug.com/40228013) Remove when on released
     * versions of the SDK.
     *
     * @return True if the current Android SDK supports {@link LocaleManager}
     */
    @ChecksSdkIntAtLeast(api = 33)
    public static boolean shouldUseSystemManagedLocale() {
        // For Android 15+, we'll use our own locale management
        if (Build.VERSION.SDK_INT >= 35) {
            return false;
        }
        // Use system locale management for Android 13-14
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU;
    }

    /**
     * Return true if the locale is an exact match for an available UI language.
     * Note: "en" and "en-AU" will return false since the available locales are "en-GB" and "en-US".
     * @param potentialUiLanguage BCP-47 language tag representing a locale (e.g. "en-US")
     */
    public static boolean isAvailableExactUiLanguage(String potentialUiLanguage) {
        return isAvailableUiLanguage(potentialUiLanguage, null);
    }

    /**
     * Return true if this locale is available or has a reasonable fallback language that can be
     * used for UI. For example we do not have language packs for "en" or "pt" but fallback to the
     * reasonable alternatives "en-US" and "pt-BR". Similarly, we have no language pack for "es-MX"
     * or "es-AR" but will use "es-419" for both. However, for languages with no translations
     * (e.g. "yo", "cy", ect.) the fallback is "en-US" which is not reasonable.
     * @param potentialUiLanguage BCP-47 language tag representing a locale (e.g. "en-US")
     */
    public static boolean isSupportedUiLanguage(String potentialUiLanguage) {
        if (potentialUiLanguage == null) {
            Log.w(TAG, "Null language provided for support check");
            return false;
        }

        try {
            // First check if it's a system language
            if (isFollowSystemLanguage(potentialUiLanguage)) {
                Log.i(TAG, "System language is always supported");
                return true;
            }

            // Check if the exact language is available
            boolean isExactMatch = isAvailableExactUiLanguage(potentialUiLanguage);
            if (isExactMatch) {
                Log.i(TAG, "Exact language match found for: " + potentialUiLanguage);
                return true;
            }

            // If not an exact match, check if base language is supported
            boolean isBaseMatch = isAvailableUiLanguage(potentialUiLanguage, BASE_LANGUAGE_COMPARATOR);
            Log.i(TAG, "Base language support check for " + potentialUiLanguage + ": " + isBaseMatch);
            return isBaseMatch;
        } catch (Exception e) {
            Log.e(TAG, "Error checking language support for: " + potentialUiLanguage, e);
            return false;
        }
    }

    private static boolean isAvailableUiLanguage(
            String potentialUiLanguage, Comparator<String> comparator) {
        // The default system language is always an available UI language.
        if (isFollowSystemLanguage(potentialUiLanguage)) return true;
        return Arrays.binarySearch(
                        ResourceBundle.getAvailableLocales(), potentialUiLanguage, comparator)
                >= 0;
    }

    /**
     * Comparator that removes any country or script information from either language tag
     * since they are not needed for locale availability checks.
     * Example: "es-MX" and "es-ES" will evaluate as equal.
     */
    private static final Comparator<String> BASE_LANGUAGE_COMPARATOR =
            new Comparator<String>() {
                @Override
                public int compare(String a, String b) {
                    String langA = LocaleUtils.toBaseLanguage(a);
                    String langB = LocaleUtils.toBaseLanguage(b);
                    return langA.compareTo(langB);
                }
            };

    /**
     * Ensures the app locale is properly applied during application startup.
     * This should be called from attachBaseContext() to ensure locale is set before any resources are loaded.
     * @param context The application context
     */
    public static void ensureAppLocaleAppliedEarly(Context context) {
        try {
            String languagePref = getAppLanguagePrefStartUp(context);
            Log.i(TAG, "Early locale application - preference: " + languagePref);
            
            if (languagePref != null) {
                // Create new configuration to avoid modifying the existing one
                Configuration config = new Configuration(context.getResources().getConfiguration());
                Locale locale = Locale.forLanguageTag(languagePref);
                
                // Set default locale first
                Locale.setDefault(locale);
                
                // Update configuration
                config.setLocale(locale);
                context.getResources().updateConfiguration(config, context.getResources().getDisplayMetrics());
                
                // For Android 13-14, also use system locale management
                if (shouldUseSystemManagedLocale()) {
                    try {
                        setSystemManagedAppLanguage(languagePref);
                    } catch (Exception e) {
                        Log.e(TAG, "Failed to set system managed locale early", e);
                    }
                }
                
                Log.i(TAG, "Successfully applied early locale: " + locale);
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to apply early locale", e);
        }
    }

    /**
     * Ensures the app locale is properly preserved during restart.
     * @param intent The intent used for restarting the application
     */
    public static void preserveLocaleForRestart(Intent intent) {
        try {
            String currentLocale = getAppLanguagePref();
            Log.i(TAG, "Preserving locale for restart: " + currentLocale);
            
            if (currentLocale != null) {
                intent.putExtra(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE, currentLocale);
                
                // For Android 13+, also preserve in LocaleManager
                if (shouldUseSystemManagedLocale()) {
                    try {
                        setSystemManagedAppLanguage(currentLocale);
                    } catch (Exception e) {
                        Log.e(TAG, "Failed to preserve system managed locale", e);
                    }
                }
                
                Log.i(TAG, "Successfully preserved locale in intent");
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to preserve locale for restart", e);
        }
    }

    /**
     * Restores the app locale from a restart intent.
     * @param intent The intent used to restart the application
     */
    public static void restoreLocaleFromRestart(Intent intent) {
        try {
            if (intent != null && intent.hasExtra(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE)) {
                String locale = intent.getStringExtra(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE);
                Log.i(TAG, "Restoring locale from restart: " + locale);
                
                if (shouldUseSystemManagedLocale()) {
                    try {
                        setSystemManagedAppLanguage(locale);
                    } catch (Exception ex) {
                        Log.e(TAG, "Failed to restore system managed locale", ex);
                    }
                }
                
                setAppLanguagePref(locale);
                Log.i(TAG, "Successfully restored locale from restart");
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to restore locale from restart", e);
        }
    }

    /**
     * Updates the application configuration with the new locale
     */
    private static void updateAppConfiguration(Context context, String languageName) {
        if (context == null) return;
        
        try {
            // Create new configuration to avoid modifying existing one
            Configuration config = new Configuration(context.getResources().getConfiguration());
            Locale newLocale = Locale.forLanguageTag(languageName != null ? languageName : 
                    Locale.getDefault().toLanguageTag());
            
            // Set default locale first
            Locale.setDefault(newLocale);
            
            // Update configuration
            config.setLocale(newLocale);
            context.getResources().updateConfiguration(
                    config, context.getResources().getDisplayMetrics());
            
            Log.i(TAG, "Updated configuration with locale: " + newLocale);
        } catch (Exception e) {
            Log.e(TAG, "Error updating configuration", e);
            throw e;
        }
    }

    /**
     * Checks if the language is supported on the current device
     */
    private static boolean isLanguageSupportedOnDevice(String languageName) {
        if (languageName == null || isFollowSystemLanguage(languageName)) {
            return true;
        }
        
        try {
            // Get available locales
            Locale[] availableLocales = Locale.getAvailableLocales();
            Locale targetLocale = Locale.forLanguageTag(languageName);
            
            // First try exact match
            for (Locale locale : availableLocales) {
                if (locale.equals(targetLocale)) {
                    Log.i(TAG, "Found exact locale match for: " + languageName);
                    return true;
                }
            }
            
            // Then try language match
            String language = targetLocale.getLanguage();
            for (Locale locale : availableLocales) {
                if (locale.getLanguage().equals(language)) {
                    Log.i(TAG, "Found language match for: " + languageName);
                    return true;
                }
            }
            
            Log.w(TAG, "Language not found in available locales: " + languageName);
            return false;
        } catch (Exception e) {
            Log.e(TAG, "Error checking language support", e);
            return false;
        }
    }
}
