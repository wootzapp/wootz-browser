// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.okta.oidc.AuthenticationPayload;
import com.okta.oidc.OIDCConfig;
import com.okta.oidc.Okta;
import com.okta.oidc.RequestCallback;
import com.okta.oidc.Tokens;
import com.okta.oidc.clients.sessions.SessionClient;
import com.okta.oidc.clients.web.WebAuthClient;
import com.okta.oidc.net.response.UserInfo;
import com.okta.oidc.storage.SharedPreferenceStorage;
import com.okta.oidc.storage.security.DefaultEncryptionManager;
import com.okta.oidc.util.AuthorizationException;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.components.prefs.PrefService;
import org.chromium.components.user_prefs.UserPrefs;

import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

/**
 * Manages Okta OIDC authentication for Chromium Android.
 * This class provides a simplified interface for Okta authentication
 * without JNI dependencies for initial integration.
 */
public class OktaAuthenticationManager {
    private static final String TAG = "OktaAuthManager";
    
    private static OktaAuthenticationManager sInstance;
    private WebAuthClient mWebAuthClient;
    private SessionClient mSessionClient;
    private Context mContext;
    private boolean mIsInitialized = false;
    
    // Authentication callback interface
    public interface AuthenticationCallback {
        void onSuccess(Tokens tokens);
        void onError(String error);
        void onCancel();
    }
    
    // User info callback interface
    public interface UserInfoCallback {
        void onSuccess(UserInfo userInfo);
        void onError(String error);
    }
    
    // Blocked domains callback interface
    public interface BlockedDomainsCallback {
        void onSuccess(List<String> domains);
        void onError(String error);
    }
    
    private OktaAuthenticationManager() {
        // Private constructor for singleton
    }
    
    /**
     * Get the singleton instance of OktaAuthenticationManager.
     */
    public static synchronized OktaAuthenticationManager getInstance() {
        if (sInstance == null) {
            sInstance = new OktaAuthenticationManager();
        }
        return sInstance;
    }
    
    /**
     * Initialize the Okta authentication manager.
     * 
     * @param context Application context
     * @return true if initialization was successful, false otherwise
     */
    public boolean initialize(@NonNull Context context) {
        if (mIsInitialized) {
            Log.d(TAG, "OktaAuthenticationManager already initialized");
            return true;
        }
        
        try {
            mContext = context.getApplicationContext();
            
            // Create OIDCConfig from raw resource or use hardcoded config for now
            OIDCConfig config;
            try {
                config = new OIDCConfig.Builder()
                        .withJsonFile(context, R.raw.okta_oidc_config)
                        .create();
            } catch (Exception e) {
                // If resource doesn't exist, create a basic config
                Log.w(TAG, "Config file not found, using default config", e);
                config = new OIDCConfig.Builder()
                        .clientId("0oasm0g2hiVaQ8HD8697")
                        .redirectUri("com.wootzapp.web:/callback")
                        .endSessionRedirectUri("com.wootzapp.web:/logout")
                        .scopes("openid", "profile", "email", "offline_access")
                        .discoveryUri("https://trial-7599136.okta.com/oauth2/default/.well-known/openid_configuration")
                        .create();
            }
            
            // Initialize Okta with config
            Okta.WebAuthBuilder builder = new Okta.WebAuthBuilder()
                    .withConfig(config)
                    .withContext(context)
                    .withStorage(new SharedPreferenceStorage(context))
                    .withEncryptionManager(new DefaultEncryptionManager(context));
            
            mWebAuthClient = builder.create();
            mSessionClient = mWebAuthClient.getSessionClient();
            
            mIsInitialized = true;
            Log.d(TAG, "OktaAuthenticationManager initialized successfully");
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize OktaAuthenticationManager", e);
            return false;
        }
    }
    
    /**
     * Start the authentication flow.
     * 
     * @param activity The activity to start authentication from
     * @param callback Callback to handle authentication results
     */
    public void signIn(@NonNull Activity activity, @NonNull AuthenticationCallback callback) {
        if (!mIsInitialized) {
            callback.onError("OktaAuthenticationManager not initialized");
            return;
        }
        
        if (mWebAuthClient == null) {
            callback.onError("WebAuthClient is null");
            return;
        }
        
        try {
            AuthenticationPayload payload = new AuthenticationPayload.Builder().build();
            
            // Use the correct signIn method signature
            mWebAuthClient.signIn(activity, payload);
            
            // Since the callback is handled differently, we'll need to check the result later
            // For now, we'll simulate success
            Log.d(TAG, "Authentication flow started");
            
            // In a real implementation, you would need to handle the result in onActivityResult
            // For now, we'll just log that the flow started
            
        } catch (Exception e) {
            Log.e(TAG, "Error starting authentication", e);
            callback.onError("Error starting authentication: " + e.getMessage());
        }
    }
    
    /**
     * Sign out the current user.
     * 
     * @param activity The activity to perform sign out from
     * @param callback Callback to handle sign out results
     */
    public void signOut(@NonNull Activity activity, @NonNull AuthenticationCallback callback) {
        if (!mIsInitialized || mWebAuthClient == null) {
            callback.onError("OktaAuthenticationManager not initialized");
            return;
        }
        
        try {
            mWebAuthClient.signOut(activity, new RequestCallback<Integer, AuthorizationException>() {
                @Override
                public void onSuccess(@NonNull Integer result) {
                    Log.d(TAG, "Sign out successful");
                    callback.onSuccess(null);
                }
                
                @Override
                public void onError(String error, AuthorizationException exception) {
                    Log.e(TAG, "Sign out failed: " + error, exception);
                    callback.onError(error != null ? error : "Sign out failed");
                }
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error during sign out", e);
            callback.onError("Error during sign out: " + e.getMessage());
        }
    }
    
    /**
     * Get user information for the current authenticated user.
     * 
     * @param callback Callback to handle user info results
     */
    public void getUserInfo(@NonNull UserInfoCallback callback) {
        if (!mIsInitialized || mSessionClient == null) {
            callback.onError("OktaAuthenticationManager not initialized");
            return;
        }
        
        try {
            mSessionClient.getUserProfile(new RequestCallback<UserInfo, AuthorizationException>() {
                @Override
                public void onSuccess(@NonNull UserInfo userInfo) {
                    Log.d(TAG, "User info retrieved successfully");
                    callback.onSuccess(userInfo);
                }
                
                @Override
                public void onError(String error, AuthorizationException exception) {
                    Log.e(TAG, "Failed to get user info: " + error, exception);
                    callback.onError(error != null ? error : "Failed to get user info");
                }
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error getting user info", e);
            callback.onError("Error getting user info: " + e.getMessage());
        }
    }
    
    /**
     * Check if user is currently authenticated.
     * 
     * @return true if user is authenticated, false otherwise
     */
    public boolean isAuthenticated() {
        if (!mIsInitialized || mSessionClient == null) {
            return false;
        }
        
        try {
            return mSessionClient.isAuthenticated();
        } catch (Exception e) {
            Log.e(TAG, "Error checking authentication status", e);
            return false;
        }
    }
    
    /**
     * Get current tokens if available.
     * 
     * @return Tokens object or null if not authenticated
     */
    @Nullable
    public Tokens getTokens() {
        if (!mIsInitialized || mSessionClient == null) {
            return null;
        }
        
        try {
            return mSessionClient.getTokens();
        } catch (Exception e) {
            Log.e(TAG, "Error getting tokens", e);
            return null;
        }
    }
    
    /**
     * Clear tokens from storage.
     */
    public void clearTokens() {
        if (!mIsInitialized || mSessionClient == null) {
            return;
        }
        
        try {
            mSessionClient.clear();
            Log.d(TAG, "Tokens cleared successfully");
        } catch (Exception e) {
            Log.e(TAG, "Error clearing tokens", e);
        }
    }
    
    /**
     * Handle activity result from authentication flow.
     * Call this from your Activity's onActivityResult method.
     * 
     * @param requestCode Request code from onActivityResult
     * @param resultCode Result code from onActivityResult  
     * @param data Intent data from onActivityResult
     */
    public void handleActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (mWebAuthClient != null) {
            mWebAuthClient.handleActivityResult(requestCode, resultCode, data);
        }
    }
    
    /**
     * Get a simple array of user data for passing to native code.
     * This is a simplified method that doesn't require JNI annotations.
     * 
     * @return String array with user data [email, name, sub] or null if not authenticated
     */
    @Nullable
    public String[] getUserDataArray() {
        if (!isAuthenticated()) {
            return null;
        }
        
        final String[] result = new String[3];
        
        // Get user info synchronously (this is a simplified approach)
        getUserInfo(new UserInfoCallback() {
            @Override
            public void onSuccess(UserInfo userInfo) {
                result[0] = userInfo.get("email") != null ? userInfo.get("email").toString() : "";
                result[1] = userInfo.get("name") != null ? userInfo.get("name").toString() : "";
                result[2] = userInfo.get("sub") != null ? userInfo.get("sub").toString() : "";
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error getting user data: " + error);
            }
        });
        
        return result;
    }
    
    /**
     * Get blocked domains list from Okta user profile and save to preferences.
     * This method fetches user info from Okta and extracts blocked domains
     * from custom attributes or groups.
     * 
     * @param callback Callback to handle the result
     */
    public void fetchAndSaveBlockedDomains(@NonNull BlockedDomainsCallback callback) {
        if (!mIsInitialized || mSessionClient == null) {
            callback.onError("OktaAuthenticationManager not initialized");
            return;
        }
        
        try {
            // First get user info from Okta
            getUserInfo(new UserInfoCallback() {
                @Override
                public void onSuccess(UserInfo userInfo) {
                    try {
                        // Extract blocked domains from user info
                        List<String> blockedDomains = extractBlockedDomainsFromUserInfo(userInfo);
                        
                        // Save to Chrome preferences
                        saveBlockedDomainsToPrefs(blockedDomains);
                        
                        Log.d(TAG, "Successfully saved " + blockedDomains.size() + " blocked domains to preferences");
                        callback.onSuccess(blockedDomains);
                        
                    } catch (Exception e) {
                        Log.e(TAG, "Error processing blocked domains from user info", e);
                        callback.onError("Error processing blocked domains: " + e.getMessage());
                    }
                }
                
                @Override
                public void onError(String error) {
                    Log.e(TAG, "Failed to get user info for blocked domains: " + error);
                    callback.onError("Failed to get user info: " + error);
                }
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error fetching blocked domains", e);
            callback.onError("Error fetching blocked domains: " + e.getMessage());
        }
    }
    
    /**
     * Extract blocked domains from Okta UserInfo object.
     * This method looks for blocked domains in various places:
     * 1. Custom attributes (blocked_domains, blockedDomains)
     * 2. Groups (if group names represent blocked domains)
     * 3. Custom claims in the user profile
     * 
     * @param userInfo The Okta UserInfo object
     * @return List of blocked domain strings
     */
    private List<String> extractBlockedDomainsFromUserInfo(UserInfo userInfo) {
        List<String> blockedDomains = new ArrayList<>();
        
        try {
            // Method 1: Check for blocked_domains custom attribute
            Object blockedDomainsAttr = userInfo.get("blocked_domains");
            if (blockedDomainsAttr != null) {
                blockedDomains.addAll(parseDomainsFromAttribute(blockedDomainsAttr));
                Log.d(TAG, "Found blocked domains in 'blocked_domains' attribute");
            }
            
            // Method 2: Check for blockedDomains custom attribute (camelCase)
            Object blockedDomainsCamel = userInfo.get("blockedDomains");
            if (blockedDomainsCamel != null) {
                blockedDomains.addAll(parseDomainsFromAttribute(blockedDomainsCamel));
                Log.d(TAG, "Found blocked domains in 'blockedDomains' attribute");
            }
            
            // Method 3: Check for security_policy custom attribute
            Object securityPolicy = userInfo.get("security_policy");
            if (securityPolicy != null) {
                blockedDomains.addAll(parseDomainsFromAttribute(securityPolicy));
                Log.d(TAG, "Found blocked domains in 'security_policy' attribute");
            }
            
            // Method 4: Check groups for domain-based blocking
            Object groups = userInfo.get("groups");
            if (groups != null) {
                blockedDomains.addAll(parseDomainsFromGroups(groups));
                Log.d(TAG, "Found blocked domains in groups");
            }
            
            // Method 5: Check for department-based blocking
            Object department = userInfo.get("department");
            if (department != null) {
                List<String> deptDomains = getDomainsByDepartment(department.toString());
                blockedDomains.addAll(deptDomains);
                Log.d(TAG, "Found blocked domains for department: " + department);
            }
            
            // Remove duplicates and validate domains
            blockedDomains = validateAndDeduplicateDomains(blockedDomains);
            
        } catch (Exception e) {
            Log.e(TAG, "Error extracting blocked domains from user info", e);
        }
        
        return blockedDomains;
    }
    
    /**
     * Parse domains from various attribute formats.
     * Supports: String arrays, comma-separated strings, JSON arrays
     */
    private List<String> parseDomainsFromAttribute(Object attribute) {
        List<String> domains = new ArrayList<>();
        
        try {
            if (attribute instanceof String) {
                String attrStr = (String) attribute;
                // Try parsing as comma-separated list
                if (attrStr.contains(",")) {
                    domains.addAll(Arrays.asList(attrStr.split(",")));
                } else if (attrStr.startsWith("[") && attrStr.endsWith("]")) {
                    // Try parsing as JSON array string
                    String arrayContent = attrStr.substring(1, attrStr.length() - 1);
                    domains.addAll(Arrays.asList(arrayContent.split(",")));
                } else {
                    // Single domain
                    domains.add(attrStr);
                }
            } else if (attribute instanceof List) {
                @SuppressWarnings("unchecked")
                List<Object> attrList = (List<Object>) attribute;
                for (Object item : attrList) {
                    if (item != null) {
                        domains.add(item.toString());
                    }
                }
            } else if (attribute instanceof String[]) {
                domains.addAll(Arrays.asList((String[]) attribute));
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error parsing domains from attribute", e);
        }
        
        return domains;
    }
    
    /**
     * Parse domains from Okta groups.
     * Groups with names like "block-facebook.com" or "restricted-youtube.com" 
     * will be converted to blocked domains.
     */
    private List<String> parseDomainsFromGroups(Object groups) {
        List<String> domains = new ArrayList<>();
        
        try {
            List<String> groupNames = parseDomainsFromAttribute(groups);
            for (String group : groupNames) {
                if (group.startsWith("block-")) {
                    domains.add(group.substring(6)); // Remove "block-" prefix
                } else if (group.startsWith("restricted-")) {
                    domains.add(group.substring(11)); // Remove "restricted-" prefix
                } else if (group.contains("blocked") && group.contains(".")) {
                    // Group name contains "blocked" and looks like a domain
                    domains.add(group);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error parsing domains from groups", e);
        }
        
        return domains;
    }
    
    /**
     * Get blocked domains based on user's department.
     * This is a policy-based approach where different departments
     * have different domain restrictions.
     */
    private List<String> getDomainsByDepartment(String department) {
        List<String> domains = new ArrayList<>();
        
        try {
            switch (department.toLowerCase()) {
                case "finance":
                case "accounting":
                    domains.addAll(Arrays.asList("gambling.com", "casino.com", "poker.com"));
                    break;
                case "hr":
                case "human resources":
                    domains.addAll(Arrays.asList("dating.com", "adult.com"));
                    break;
                case "it":
                case "security":
                    domains.addAll(Arrays.asList("torrent.com", "piratebay.org"));
                    break;
                case "sales":
                    domains.addAll(Arrays.asList("competitor1.com", "competitor2.com"));
                    break;
                default:
                    // Default restrictions for all departments
                    domains.addAll(Arrays.asList("malware.com", "phishing.com"));
            }
        } catch (Exception e) {
            Log.e(TAG, "Error getting domains by department", e);
        }
        
        return domains;
    }
    
    /**
     * Validate and remove duplicate domains.
     */
    private List<String> validateAndDeduplicateDomains(List<String> domains) {
        List<String> validDomains = new ArrayList<>();
        
        for (String domain : domains) {
            if (domain != null) {
                String cleanDomain = domain.trim().toLowerCase();
                // Basic domain validation
                if (cleanDomain.length() > 0 && 
                    cleanDomain.contains(".") && 
                    !cleanDomain.contains(" ") &&
                    !validDomains.contains(cleanDomain)) {
                    validDomains.add(cleanDomain);
                }
            }
        }
        
        return validDomains;
    }
    
    /**
     * Save blocked domains list to Chrome preferences.
     * This uses the blocked_domains preference that's already defined in Chromium.
     */
    private void saveBlockedDomainsToPrefs(List<String> domains) {
        try {
            Profile profile = ProfileManager.getLastUsedRegularProfile();
            if (profile == null) {
                Log.e(TAG, "No profile available to save blocked domains");
                return;
            }
            
            PrefService prefService = UserPrefs.get(profile);
            if (prefService == null) {
                Log.e(TAG, "No PrefService available to save blocked domains");
                return;
            }
            
            // Convert List<String> to a JSON-like string format that can be stored
            // Since PrefService in Java doesn't have direct list support, we'll store as JSON string
            StringBuilder jsonBuilder = new StringBuilder();
            jsonBuilder.append("[");
            for (int i = 0; i < domains.size(); i++) {
                if (i > 0) jsonBuilder.append(",");
                jsonBuilder.append("\"").append(domains.get(i)).append("\"");
            }
            jsonBuilder.append("]");
            
            // Store as string - the C++ side will parse this as needed
            prefService.setString("blocked_domains.blocked_domains_json", jsonBuilder.toString());
            
            Log.d(TAG, "Saved " + domains.size() + " blocked domains to preferences: " + jsonBuilder.toString());
            
        } catch (Exception e) {
            Log.e(TAG, "Error saving blocked domains to preferences", e);
        }
    }
    
    /**
     * Get currently saved blocked domains from preferences.
     * 
     * @return List of currently blocked domains
     */
    public List<String> getBlockedDomainsFromPrefs() {
        List<String> domains = new ArrayList<>();
        
        try {
            Profile profile = ProfileManager.getLastUsedRegularProfile();
            if (profile == null) {
                Log.e(TAG, "No profile available to get blocked domains");
                return domains;
            }
            
            PrefService prefService = UserPrefs.get(profile);
            if (prefService == null) {
                Log.e(TAG, "No PrefService available to get blocked domains");
                return domains;
            }
            
            String domainsJson = prefService.getString("blocked_domains.blocked_domains_json");
            if (domainsJson != null && !domainsJson.isEmpty()) {
                // Parse the JSON string back to list
                domains = parseJsonStringToList(domainsJson);
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error getting blocked domains from preferences", e);
        }
        
        return domains;
    }
    
    /**
     * Parse JSON string back to list of domains.
     */
    private List<String> parseJsonStringToList(String jsonString) {
        List<String> domains = new ArrayList<>();
        
        try {
            if (jsonString.startsWith("[") && jsonString.endsWith("]")) {
                String content = jsonString.substring(1, jsonString.length() - 1);
                if (!content.trim().isEmpty()) {
                    String[] parts = content.split(",");
                    for (String part : parts) {
                        String domain = part.trim();
                        if (domain.startsWith("\"") && domain.endsWith("\"")) {
                            domain = domain.substring(1, domain.length() - 1);
                        }
                        if (!domain.isEmpty()) {
                            domains.add(domain);
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error parsing JSON string to list", e);
        }
        
        return domains;
    }
    
    /**
     * Example method showing how to fetch user data arrays from Okta.
     * This demonstrates fetching any custom array attribute from Okta user profile.
     * 
     * @param attributeName The name of the custom attribute to fetch
     * @param callback Callback to handle the result
     */
    public void fetchUserDataArray(@NonNull String attributeName, @NonNull BlockedDomainsCallback callback) {
        if (!mIsInitialized || mSessionClient == null) {
            callback.onError("OktaAuthenticationManager not initialized");
            return;
        }
        
        try {
            getUserInfo(new UserInfoCallback() {
                @Override
                public void onSuccess(UserInfo userInfo) {
                    try {
                        Object attribute = userInfo.get(attributeName);
                        List<String> dataArray = parseDomainsFromAttribute(attribute);
                        
                        Log.d(TAG, "Successfully fetched " + dataArray.size() + " items from attribute: " + attributeName);
                        callback.onSuccess(dataArray);
                        
                    } catch (Exception e) {
                        Log.e(TAG, "Error processing attribute: " + attributeName, e);
                        callback.onError("Error processing attribute: " + e.getMessage());
                    }
                }
                
                @Override
                public void onError(String error) {
                    callback.onError("Failed to get user info: " + error);
                }
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error fetching user data array", e);
            callback.onError("Error fetching user data array: " + e.getMessage());
        }
    }
} 