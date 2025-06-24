// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.components.prefs.PrefService;
import org.chromium.components.user_prefs.UserPrefs;

import java.util.List;

/**
 * Bridge class that simplifies the integration between Okta authentication
 * and Chrome's blocked domains preferences system.
 * 
 * This class provides high-level methods to:
 * 1. Authenticate with Okta
 * 2. Fetch blocked domains from Okta user profile
 * 3. Save blocked domains to Chrome preferences
 * 4. Manage the integration lifecycle
 */
public class OktaDomainBlockingBridge {
    private static final String TAG = "OktaDomainBridge";
    
    private static OktaDomainBlockingBridge sInstance;
    private OktaAuthenticationManager mOktaManager;
    private Context mContext;
    private boolean mIsInitialized = false;
    
    // Callback interface for the complete flow
    public interface DomainBlockingCallback {
        void onSuccess(List<String> blockedDomains);
        void onError(String error);
        void onAuthenticationRequired();
    }
    
    // Callback interface for authentication status
    public interface AuthenticationStatusCallback {
        void onAuthenticated();
        void onNotAuthenticated();
        void onError(String error);
    }
    
    private OktaDomainBlockingBridge() {
        // Private constructor for singleton
    }
    
    /**
     * Get the singleton instance of OktaDomainBlockingBridge.
     */
    public static synchronized OktaDomainBlockingBridge getInstance() {
        if (sInstance == null) {
            sInstance = new OktaDomainBlockingBridge();
        }
        return sInstance;
    }
    
    /**
     * Initialize the bridge with application context.
     * 
     * @param context Application context
     * @return true if initialization was successful, false otherwise
     */
    public boolean initialize(@NonNull Context context) {
        if (mIsInitialized) {
            Log.d(TAG, "OktaDomainBlockingBridge already initialized");
            return true;
        }
        
        try {
            mContext = context.getApplicationContext();
            mOktaManager = OktaAuthenticationManager.getInstance();
            
            if (!mOktaManager.initialize(mContext)) {
                Log.e(TAG, "Failed to initialize OktaAuthenticationManager");
                return false;
            }
            
            mIsInitialized = true;
            Log.d(TAG, "OktaDomainBlockingBridge initialized successfully");
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize OktaDomainBlockingBridge", e);
            return false;
        }
    }
    
    /**
     * Complete flow: Authenticate with Okta and fetch blocked domains.
     * This is the main method to use for the complete integration.
     * 
     * @param activity Activity for authentication UI
     * @param callback Callback to handle the complete flow result
     */
    public void authenticateAndFetchBlockedDomains(@NonNull Activity activity, 
                                                  @NonNull DomainBlockingCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaDomainBlockingBridge not initialized");
            return;
        }
        
        // First check if already authenticated
        if (mOktaManager.isAuthenticated()) {
            Log.d(TAG, "User already authenticated, fetching blocked domains");
            fetchBlockedDomainsFromOkta(callback);
            return;
        }
        
        // Need to authenticate first
        Log.d(TAG, "User not authenticated, starting authentication flow");
        mOktaManager.signIn(activity, new OktaAuthenticationManager.AuthenticationCallback() {
            @Override
            public void onSuccess(com.okta.oidc.Tokens tokens) {
                Log.d(TAG, "Authentication successful, now fetching blocked domains");
                fetchBlockedDomainsFromOkta(callback);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Authentication failed: " + error);
                callback.onError("Authentication failed: " + error);
            }
            
            @Override
            public void onCancel() {
                Log.d(TAG, "Authentication cancelled by user");
                callback.onError("Authentication cancelled by user");
            }
        });
    }
    
    /**
     * Fetch blocked domains from Okta (user must be authenticated first).
     * 
     * @param callback Callback to handle the result
     */
    public void fetchBlockedDomainsFromOkta(@NonNull DomainBlockingCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaDomainBlockingBridge not initialized");
            return;
        }
        
        if (!mOktaManager.isAuthenticated()) {
            Log.w(TAG, "User not authenticated, cannot fetch blocked domains");
            callback.onAuthenticationRequired();
            return;
        }
        
        mOktaManager.fetchAndSaveBlockedDomains(new OktaAuthenticationManager.BlockedDomainsCallback() {
            @Override
            public void onSuccess(List<String> domains) {
                Log.d(TAG, "Successfully fetched and saved " + domains.size() + " blocked domains");
                callback.onSuccess(domains);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Failed to fetch blocked domains: " + error);
                callback.onError("Failed to fetch blocked domains: " + error);
            }
        });
    }
    
    /**
     * Check authentication status.
     * 
     * @param callback Callback to handle the authentication status
     */
    public void checkAuthenticationStatus(@NonNull AuthenticationStatusCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaDomainBlockingBridge not initialized");
            return;
        }
        
        try {
            if (mOktaManager.isAuthenticated()) {
                callback.onAuthenticated();
            } else {
                callback.onNotAuthenticated();
            }
        } catch (Exception e) {
            Log.e(TAG, "Error checking authentication status", e);
            callback.onError("Error checking authentication status: " + e.getMessage());
        }
    }
    
    /**
     * Get currently saved blocked domains from Chrome preferences.
     * 
     * @return List of currently blocked domains
     */
    public List<String> getCurrentBlockedDomains() {
        if (!mIsInitialized) {
            Log.e(TAG, "OktaDomainBlockingBridge not initialized");
            return new java.util.ArrayList<>();
        }
        
        return mOktaManager.getBlockedDomainsFromPrefs();
    }
    
    /**
     * Manually save a list of domains to blocked domains preferences.
     * This can be used for testing or manual domain management.
     * 
     * @param domains List of domains to block
     * @return true if saved successfully, false otherwise
     */
    public boolean saveBlockedDomains(@NonNull List<String> domains) {
        if (!mIsInitialized) {
            Log.e(TAG, "OktaDomainBlockingBridge not initialized");
            return false;
        }
        
        try {
            Profile profile = ProfileManager.getLastUsedRegularProfile();
            if (profile == null) {
                Log.e(TAG, "No profile available to save blocked domains");
                return false;
            }
            
            PrefService prefService = UserPrefs.get(profile);
            if (prefService == null) {
                Log.e(TAG, "No PrefService available to save blocked domains");
                return false;
            }
            
            // Convert List<String> to JSON string format
            StringBuilder jsonBuilder = new StringBuilder();
            jsonBuilder.append("[");
            for (int i = 0; i < domains.size(); i++) {
                if (i > 0) jsonBuilder.append(",");
                jsonBuilder.append("\"").append(domains.get(i)).append("\"");
            }
            jsonBuilder.append("]");
            
            // Save to preferences
            prefService.setString("blocked_domains.blocked_domains_json", jsonBuilder.toString());
            
            Log.d(TAG, "Manually saved " + domains.size() + " blocked domains to preferences");
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Error manually saving blocked domains", e);
            return false;
        }
    }
    
    /**
     * Sign out from Okta.
     * 
     * @param activity Activity for sign out UI
     * @param callback Callback to handle sign out result
     */
    public void signOut(@NonNull Activity activity, @NonNull AuthenticationStatusCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaDomainBlockingBridge not initialized");
            return;
        }
        
        mOktaManager.signOut(activity, new OktaAuthenticationManager.AuthenticationCallback() {
            @Override
            public void onSuccess(com.okta.oidc.Tokens tokens) {
                Log.d(TAG, "Sign out successful");
                callback.onAuthenticated(); // This might be confusing, but it means the operation succeeded
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Sign out failed: " + error);
                callback.onError("Sign out failed: " + error);
            }
            
            @Override
            public void onCancel() {
                Log.d(TAG, "Sign out cancelled");
                callback.onError("Sign out cancelled");
            }
        });
    }
    
    /**
     * Fetch any custom array attribute from Okta user profile.
     * This is a generic method for fetching arrays from Okta.
     * 
     * @param attributeName Name of the custom attribute to fetch
     * @param callback Callback to handle the result
     */
    public void fetchCustomArrayFromOkta(@NonNull String attributeName, 
                                       @NonNull DomainBlockingCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaDomainBlockingBridge not initialized");
            return;
        }
        
        if (!mOktaManager.isAuthenticated()) {
            Log.w(TAG, "User not authenticated, cannot fetch custom array");
            callback.onAuthenticationRequired();
            return;
        }
        
        mOktaManager.fetchUserDataArray(attributeName, new OktaAuthenticationManager.BlockedDomainsCallback() {
            @Override
            public void onSuccess(List<String> dataArray) {
                Log.d(TAG, "Successfully fetched " + dataArray.size() + " items from attribute: " + attributeName);
                callback.onSuccess(dataArray);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Failed to fetch custom array from attribute " + attributeName + ": " + error);
                callback.onError("Failed to fetch custom array: " + error);
            }
        });
    }
    
    /**
     * Get the underlying OktaAuthenticationManager instance.
     * Use this if you need access to lower-level Okta functionality.
     * 
     * @return OktaAuthenticationManager instance
     */
    public OktaAuthenticationManager getOktaManager() {
        return mOktaManager;
    }
    
    /**
     * Check if the bridge is initialized.
     * 
     * @return true if initialized, false otherwise
     */
    public boolean isInitialized() {
        return mIsInitialized;
    }
} 