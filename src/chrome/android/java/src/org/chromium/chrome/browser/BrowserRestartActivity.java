// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Process;
import android.util.Log;

import org.chromium.base.ContextUtils;
import org.chromium.base.IntentUtils;
import org.chromium.chrome.browser.language.AppLocaleUtils;
import org.chromium.chrome.browser.preferences.ChromePreferenceKeys;

import java.util.Locale;

/**
 * Kills and (optionally) restarts the main Chrome process, then immediately kills itself.
 *
 * Starting this Activity should only be done by the
 * {@link org.chromium.chrome.browser.init.ChromeLifetimeController}, and requires
 * passing in the process ID (the Intent should have the value of Process#myPid() as an extra).
 *
 * This Activity runs on a separate process from the main Chrome browser and cannot see the main
 * process' Activities.  It works around an Android framework issue for alarms set via the
 * AlarmManager, which requires a minimum alarm duration of 5 seconds: https://crbug.com/515919.
 */
public class BrowserRestartActivity extends Activity {
    public static final String EXTRA_MAIN_PID =
            "org.chromium.chrome.browser.BrowserRestartActivity.main_pid";
    public static final String EXTRA_RESTART =
            "org.chromium.chrome.browser.BrowserRestartActivity.restart";

    private static final String TAG = "BrowserRestartActivity";

    /**
     * Creates an Intent to start the {@link BrowserRestartActivity}.  Must only be called by the
     * {@link org.chromium.chrome.browser.init.ChromeLifetimeController}.
     * @param context       Context to use when constructing the Intent.
     * @param restartChrome Whether or not to restart Chrome after killing the process.
     * @return Intent that can be used to restart Chrome.
     */
    public static Intent createIntent(Context context, boolean restartChrome) {
        Intent intent = new Intent();
        intent.setClassName(context.getPackageName(), BrowserRestartActivity.class.getName());
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(BrowserRestartActivity.EXTRA_MAIN_PID, Process.myPid());
        intent.putExtra(BrowserRestartActivity.EXTRA_RESTART, restartChrome);
        return intent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "BrowserRestartActivity onCreate");

        // Kill the main Chrome process.
        Intent intent = getIntent();
        int mainBrowserPid =
                IntentUtils.safeGetIntExtra(intent, BrowserRestartActivity.EXTRA_MAIN_PID, -1);
        assert mainBrowserPid != -1;
        assert mainBrowserPid != Process.myPid();
        
        Log.i(TAG, "Killing main browser process: " + mainBrowserPid);
        Process.killProcess(mainBrowserPid);

        // Fire an Intent to restart Chrome, if necessary.
        boolean restart =
                IntentUtils.safeGetBooleanExtra(
                        intent, BrowserRestartActivity.EXTRA_RESTART, false);
        
        Log.i(TAG, "Should restart: " + restart);
        
        if (restart) {
            Context context = ContextUtils.getApplicationContext();
            Intent restartIntent = new Intent(Intent.ACTION_MAIN);
            restartIntent.setPackage(context.getPackageName());
            restartIntent.addCategory(Intent.CATEGORY_LAUNCHER);
            restartIntent.setFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK |
                    Intent.FLAG_ACTIVITY_CLEAR_TASK |
                    Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED |
                    Intent.FLAG_ACTIVITY_CLEAR_TOP);
            
            // Preserve locale information in restart intent
            try {
                String currentLocale = AppLocaleUtils.getAppLanguagePref();
                Log.i(TAG, "Current locale before restart: " + currentLocale);
                if (currentLocale != null) {
                    // Add locale to intent
                    restartIntent.putExtra(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE, currentLocale);
                    
                    // Update configuration before restart
                    try {
                        Configuration config = new Configuration(context.getResources().getConfiguration());
                        Locale newLocale = Locale.forLanguageTag(currentLocale);
                        Locale.setDefault(newLocale);
                        config.setLocale(newLocale);
                        context.getResources().updateConfiguration(
                                config, context.getResources().getDisplayMetrics());
                        Log.i(TAG, "Updated configuration before restart with locale: " + newLocale);
                    } catch (Exception e) {
                        Log.e(TAG, "Error updating configuration before restart", e);
                    }
                    
                    // For Android 13+, ensure system locale is set
                    if (AppLocaleUtils.shouldUseSystemManagedLocale()) {
                        try {
                            AppLocaleUtils.setSystemManagedAppLanguage(currentLocale);
                            Log.i(TAG, "Updated system managed locale before restart");
                        } catch (Exception e) {
                            Log.e(TAG, "Error updating system managed locale before restart", e);
                        }
                    }
                    
                    Log.i(TAG, "Preserved locale in restart intent: " + currentLocale);
                }
            } catch (Exception e) {
                Log.e(TAG, "Error preserving locale for restart", e);
            }
            
            try {
                Log.i(TAG, "Starting new Chrome process with preserved locale");
                context.startActivity(restartIntent);
            } catch (Exception e) {
                Log.e(TAG, "Error starting new Chrome process", e);
                // Try fallback restart with preserved locale
                try {
                    restartIntent = context.getPackageManager()
                            .getLaunchIntentForPackage(context.getPackageName());
                    if (restartIntent != null) {
                        Log.i(TAG, "Attempting fallback restart");
                        // Preserve locale in fallback intent
                        String currentLocale = AppLocaleUtils.getAppLanguagePref();
                        if (currentLocale != null) {
                            restartIntent.putExtra(ChromePreferenceKeys.APPLICATION_OVERRIDE_LANGUAGE, currentLocale);
                        }
                        restartIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                                Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        context.startActivity(restartIntent);
                    }
                } catch (Exception ex) {
                    Log.e(TAG, "Error during fallback restart", ex);
                }
            }
        }

        // Kill this process.
        Log.i(TAG, "Finishing BrowserRestartActivity");
        finish();
        Process.killProcess(Process.myPid());
    }
}
