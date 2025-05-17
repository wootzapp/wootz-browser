// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Application;
import android.content.res.Configuration;
import android.util.Log;

import androidx.annotation.Nullable;

import io.branch.referral.Branch;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.version_info.Channel;
import org.chromium.base.version_info.VersionConstants;
import org.chromium.build.BuildConfig;
import org.chromium.chrome.browser.accessibility.hierarchysnapshotter.HierarchySnapshotter;
import org.chromium.chrome.browser.app.notifications.ContextualNotificationPermissionRequesterImpl;
import org.chromium.chrome.browser.background_task_scheduler.ChromeBackgroundTaskFactory;
import org.chromium.chrome.browser.base.SplitCompatApplication;
import org.chromium.chrome.browser.crash.ChromePureJavaExceptionReporter;
import org.chromium.chrome.browser.customtabs.CustomTabsConnection;
import org.chromium.chrome.browser.dependency_injection.ChromeAppComponent;
import org.chromium.chrome.browser.dependency_injection.ChromeAppModule;
import org.chromium.chrome.browser.dependency_injection.DaggerChromeAppComponent;
import org.chromium.chrome.browser.dependency_injection.ModuleFactoryOverrides;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.fonts.FontPreloader;
import org.chromium.chrome.browser.night_mode.SystemNightModeMonitor;
import org.chromium.chrome.browser.profiles.ProfileResolver;
import org.chromium.chrome.browser.webauthn.CredManUiRecommenderImpl;
import org.chromium.components.browser_ui.util.BrowserUiUtilsCachedFlags;
import org.chromium.components.browser_ui.util.GlobalDiscardableReferencePool;
import org.chromium.components.embedder_support.browser_context.PartitionResolverSupplier;
import org.chromium.components.module_installer.util.ModuleUtil;
import org.chromium.components.webauthn.cred_man.CredManUiRecommenderProvider;
import org.chromium.url.GURL;
import android.content.Context;

/**
 * Basic application functionality that should be shared among all browser applications that use
 * chrome layer.
 *
 * Note: All application logic should be added to {@link ChromeApplicationImpl}, which will be
 * called from the superclass. See {@link SplitCompatApplication} for more info.
 */
public class ChromeApplicationImpl extends SplitCompatApplication.Impl {
    /** Lock on creation of sComponent. */
    private static final Object sLock = new Object();
    private static final String TAG = "ChromeApplicationImpl";

    @Nullable private static volatile ChromeAppComponent sComponent;

    public ChromeApplicationImpl() {}

    @Override
    public void onCreate() {
        super.onCreate();

        if (SplitCompatApplication.isBrowserProcess()) {
            Log.e(TAG, "Initializing Chrome browser process");
            
            // Initialize Branch SDK first
            try {
                Log.e(TAG, "Initializing Branch SDK");
                
                // For Android 9, we need to set the user agent string before initialization
                if (android.os.Build.VERSION.SDK_INT == android.os.Build.VERSION_CODES.P) {
                    Log.e(TAG, "Setting user agent string for Android 9");
                    
                    // Set system user agent property to avoid WebView PacProcessor crash on Android 9
                    System.setProperty("http.agent", "Mozilla/5.0 (Linux; Android 9; Mobile) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Mobile Safari/537.36");
                    
                    // Tell Branch SDK to use synchronous user agent fetching on Android 9
                    // This avoids the coroutine that causes the crash
                    Branch.setIsUserAgentSync(true);
                }
                
                // Initialize Branch SDK as normal
                io.branch.referral.Branch.enableLogging();
                io.branch.referral.Branch branch = io.branch.referral.Branch.getAutoInstance(getApplication());
                Log.e(TAG, "Branch SDK initialized successfully");
                
            } catch (Exception e) {
                Log.e(TAG, "Error initializing Branch SDK: " + e.getMessage(), e);
            }
            
            // Check Android version
            checkAndroidVersion();
            
            // Check for Google Play Services availability
            checkGooglePlayServices();

            FontPreloader.getInstance().load(getApplication());

            // Registers the extensions for all protos which would be in the Chrome split, whether
            // or not we are actually building with splits.
            AppHooks.get().registerProtoExtensions();

            // TODO(crbug.com/40266922): Remove this after code changes allow for //components to
            // access cached flags.
            BrowserUiUtilsCachedFlags.getInstance()
                    .setVerticalAutomotiveBackButtonToolbarFlag(
                            ChromeFeatureList.sVerticalAutomotiveBackButtonToolbar.isEnabled());
            BrowserUiUtilsCachedFlags.getInstance()
                    .setAsyncNotificationManagerFlag(
                            ChromeFeatureList.sAsyncNotificationManager.isEnabled());

            // Only load the native library early for bundle builds since some tests use the
            // "--disable-native-initialization" switch, and the CommandLine is not initialized at
            // this point to check.
            if (ProductConfig.IS_BUNDLE) {
                // Kick off library loading in a separate thread so it's ready when we need it.
                new Thread(() -> LibraryLoader.getInstance().ensureInitialized()).start();
            }

            // Initializes the support for dynamic feature modules (browser only).
            ModuleUtil.initApplication();

            if (VersionConstants.CHANNEL == Channel.CANARY) {
                GURL.setReportDebugThrowableCallback(
                        ChromePureJavaExceptionReporter::reportJavaException);
            }

            // Set Chrome factory for mapping BackgroundTask classes to TaskIds.
            ChromeBackgroundTaskFactory.setAsDefault();
            ContextualNotificationPermissionRequesterImpl.initialize();
            PartitionResolverSupplier.setInstance(new ProfileResolver());

            AppHooks.get().getChimeDelegate().initialize();

            // Initialize the AccessibilityHierarchySnapshotter. Do not include in release builds.
            if (!BuildConfig.IS_CHROME_BRANDED) {
                HierarchySnapshotter.initialize();
            }

            // Provide the supplier for CredManUiRecommender. This is set only for Chrome.
            CredManUiRecommenderProvider.getOrCreate()
                    .setCredManUiRecommenderSupplier(() -> new CredManUiRecommenderImpl());
        }
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        if (isSevereMemorySignal(level)
                && GlobalDiscardableReferencePool.getReferencePool() != null) {
            GlobalDiscardableReferencePool.getReferencePool().drain();
        }
        CustomTabsConnection.onTrimMemory(level);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // TODO(huayinz): Add observer pattern for application configuration changes.
        if (SplitCompatApplication.isBrowserProcess()) {
            SystemNightModeMonitor.getInstance().onApplicationConfigurationChanged();
        }
    }

    /**
     * Determines whether the given memory signal is considered severe.
     * @param level The type of signal as defined in {@link android.content.ComponentCallbacks2}.
     */
    public static boolean isSevereMemorySignal(int level) {
        // The conditions are expressed using ranges to capture intermediate levels possibly added
        // to the API in the future.
        return (level >= Application.TRIM_MEMORY_RUNNING_LOW
                        && level < Application.TRIM_MEMORY_UI_HIDDEN)
                || level >= Application.TRIM_MEMORY_MODERATE;
    }

    /** Returns the application-scoped component. */
    public static ChromeAppComponent getComponent() {
        if (sComponent == null) {
            synchronized (sLock) {
                if (sComponent == null) {
                    sComponent = createComponent();
                }
            }
        }
        return sComponent;
    }

    private static ChromeAppComponent createComponent() {
        ChromeAppModule.Factory overriddenFactory =
                ModuleFactoryOverrides.getOverrideFor(ChromeAppModule.Factory.class);
        ChromeAppModule module =
                overriddenFactory == null ? new ChromeAppModule() : overriddenFactory.create();

        AppHooksModule.Factory appHooksFactory =
                ModuleFactoryOverrides.getOverrideFor(AppHooksModule.Factory.class);
        AppHooksModule appHooksModule =
                appHooksFactory == null ? new AppHooksModule() : appHooksFactory.create();

        return DaggerChromeAppComponent.builder()
                .chromeAppModule(module)
                .appHooksModule(appHooksModule)
                .build();
    }

    private void checkGooglePlayServices() {
        Log.i(TAG, "Checking Google Play Services");
        try {
            // Store the result for later use by activities
            int result = com.google.android.gms.common.GoogleApiAvailability.getInstance()
                    .isGooglePlayServicesAvailable(getApplication());
            
            // Save this result in SharedPreferences so activities can access it
            android.content.SharedPreferences prefs = getApplication()
                    .getSharedPreferences("chrome_gms_prefs", Context.MODE_PRIVATE);
            prefs.edit().putInt("gms_availability", result).apply();
            
            Log.i(TAG, "Google Play Services availability check result: " + result);
            
        } catch (Exception e) {
            Log.e(TAG, "Error checking Google Play Services: " + e.getMessage());
            // Save error state
            android.content.SharedPreferences prefs = getApplication()
                    .getSharedPreferences("chrome_gms_prefs", Context.MODE_PRIVATE);
            prefs.edit().putInt("gms_availability", 
                    com.google.android.gms.common.ConnectionResult.SERVICE_MISSING).apply();
        }
    }

    private void checkAndroidVersion() {
        Log.i(TAG, "Checking Android version");
        
        try {
            boolean isSupported = android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P;
            Log.i(TAG, "Android version check: " + (isSupported ? "supported" : "unsupported") + 
                       " (running " + android.os.Build.VERSION.RELEASE + ")");
            if (!isSupported) {
                Log.e(TAG, "Android version is not supported: " + android.os.Build.VERSION.RELEASE);
                android.widget.Toast.makeText(
                        getApplication(),
                        "This app requires Android 9.0 or higher.",
                        android.widget.Toast.LENGTH_LONG).show();
            }
        } catch (Exception e) {
            Log.e(TAG, "Error checking Android version: " + e.getMessage());
        }
    }
}
