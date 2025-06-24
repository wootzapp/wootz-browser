// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import java.util.Arrays;
import java.util.List;

/**
 * Example class demonstrating how to integrate Okta authentication
 * with Chrome's blocked domains preferences system.
 * 
 * This class shows various usage patterns and best practices for:
 * 1. Getting arrays/lists from Okta user profile
 * 2. Saving those lists to Chrome's blocked domains preferences
 * 3. Managing the authentication and data flow
 */
public class OktaUsageExample {
    private static final String TAG = "OktaUsageExample";
    
    /**
     * Example 1: Simple integration - Authenticate and fetch blocked domains
     * This is the most common use case.
     */
    public static void simpleBlockedDomainsIntegration(@NonNull Activity activity) {
        Log.d(TAG, "Starting simple blocked domains integration");
        
        // Initialize the bridge
        OktaDomainBlockingBridge bridge = OktaDomainBlockingBridge.getInstance();
        if (!bridge.initialize(activity.getApplicationContext())) {
            Log.e(TAG, "Failed to initialize OktaDomainBlockingBridge");
            return;
        }
        
        // Authenticate and fetch blocked domains in one call
        bridge.authenticateAndFetchBlockedDomains(activity, new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> blockedDomains) {
                Log.d(TAG, "Successfully fetched " + blockedDomains.size() + " blocked domains:");
                for (String domain : blockedDomains) {
                    Log.d(TAG, "  - " + domain);
                }
                
                // Domains are automatically saved to Chrome preferences
                // You can now use them in your domain blocking logic
                handleBlockedDomains(blockedDomains);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Failed to fetch blocked domains: " + error);
                // Handle error - maybe show user notification or use default policy
                handleBlockedDomainsError(error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required but user not authenticated");
                // This shouldn't happen in this flow, but handle gracefully
            }
        });
    }
    
    /**
     * Example 2: Step-by-step integration with authentication check
     * This gives you more control over the flow.
     */
    public static void stepByStepIntegration(@NonNull Activity activity) {
        Log.d(TAG, "Starting step-by-step integration");
        
        OktaDomainBlockingBridge bridge = OktaDomainBlockingBridge.getInstance();
        if (!bridge.initialize(activity.getApplicationContext())) {
            Log.e(TAG, "Failed to initialize bridge");
            return;
        }
        
        // Step 1: Check if user is already authenticated
        bridge.checkAuthenticationStatus(new OktaDomainBlockingBridge.AuthenticationStatusCallback() {
            @Override
            public void onAuthenticated() {
                Log.d(TAG, "User is already authenticated, fetching blocked domains");
                // User is authenticated, proceed to fetch domains
                fetchDomainsWhenAuthenticated(bridge);
            }
            
            @Override
            public void onNotAuthenticated() {
                Log.d(TAG, "User not authenticated, starting authentication");
                // Need to authenticate first
                authenticateUser(activity, bridge);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error checking authentication status: " + error);
                handleAuthenticationError(error);
            }
        });
    }
    
    /**
     * Example 3: Fetching custom arrays from Okta
     * This shows how to fetch any custom array attribute from Okta user profile.
     */
    public static void fetchCustomArraysExample(@NonNull Activity activity) {
        Log.d(TAG, "Starting custom arrays example");
        
        OktaDomainBlockingBridge bridge = OktaDomainBlockingBridge.getInstance();
        if (!bridge.initialize(activity.getApplicationContext())) {
            Log.e(TAG, "Failed to initialize bridge");
            return;
        }
        
        // Ensure user is authenticated first
        bridge.authenticateAndFetchBlockedDomains(activity, new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> blockedDomains) {
                Log.d(TAG, "Authentication successful, now fetching custom arrays");
                
                // Fetch different types of custom arrays from Okta
                fetchAllowedApplications(bridge);
                fetchSecurityPolicies(bridge);
                fetchUserGroups(bridge);
                fetchDepartmentRestrictions(bridge);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Failed to authenticate for custom arrays: " + error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required for custom arrays");
            }
        });
    }
    
    /**
     * Example 4: Manual domain management
     * This shows how to manually manage blocked domains without Okta.
     */
    public static void manualDomainManagementExample(@NonNull Context context) {
        Log.d(TAG, "Starting manual domain management example");
        
        OktaDomainBlockingBridge bridge = OktaDomainBlockingBridge.getInstance();
        if (!bridge.initialize(context)) {
            Log.e(TAG, "Failed to initialize bridge");
            return;
        }
        
        // Example: Manually create a list of domains to block
        List<String> manualBlockedDomains = Arrays.asList(
            "facebook.com",
            "twitter.com", 
            "instagram.com",
            "tiktok.com",
            "youtube.com"
        );
        
        // Save them to Chrome preferences
        boolean success = bridge.saveBlockedDomains(manualBlockedDomains);
        if (success) {
            Log.d(TAG, "Successfully saved manual blocked domains");
            
            // Get them back to verify
            List<String> savedDomains = bridge.getCurrentBlockedDomains();
            Log.d(TAG, "Retrieved " + savedDomains.size() + " saved domains");
        } else {
            Log.e(TAG, "Failed to save manual blocked domains");
        }
    }
    
    /**
     * Example 5: Periodic sync with Okta
     * This shows how to periodically sync blocked domains from Okta.
     */
    public static void periodicSyncExample(@NonNull Activity activity) {
        Log.d(TAG, "Starting periodic sync example");
        
        OktaDomainBlockingBridge bridge = OktaDomainBlockingBridge.getInstance();
        if (!bridge.initialize(activity.getApplicationContext())) {
            Log.e(TAG, "Failed to initialize bridge");
            return;
        }
        
        // Check if user is authenticated before syncing
        bridge.checkAuthenticationStatus(new OktaDomainBlockingBridge.AuthenticationStatusCallback() {
            @Override
            public void onAuthenticated() {
                Log.d(TAG, "User authenticated, performing periodic sync");
                
                // Fetch latest blocked domains from Okta
                bridge.fetchBlockedDomainsFromOkta(new OktaDomainBlockingBridge.DomainBlockingCallback() {
                    @Override
                    public void onSuccess(List<String> domains) {
                        Log.d(TAG, "Periodic sync successful: " + domains.size() + " domains");
                        // Domains are automatically saved to preferences
                        notifyDomainPolicyUpdated(domains);
                    }
                    
                    @Override
                    public void onError(String error) {
                        Log.e(TAG, "Periodic sync failed: " + error);
                        // Continue with existing policy
                    }
                    
                    @Override
                    public void onAuthenticationRequired() {
                        Log.w(TAG, "Authentication expired during sync");
                        // Re-authenticate if needed
                        bridge.authenticateAndFetchBlockedDomains(activity, this);
                    }
                });
            }
            
            @Override
            public void onNotAuthenticated() {
                Log.d(TAG, "User not authenticated for periodic sync");
                // Skip sync or prompt for authentication
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error during periodic sync auth check: " + error);
            }
        });
    }
    
    // Helper methods for the examples
    
    private static void fetchDomainsWhenAuthenticated(OktaDomainBlockingBridge bridge) {
        bridge.fetchBlockedDomainsFromOkta(new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> domains) {
                Log.d(TAG, "Fetched domains when authenticated: " + domains.size());
                handleBlockedDomains(domains);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error fetching domains when authenticated: " + error);
                handleBlockedDomainsError(error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.e(TAG, "Authentication required but user should be authenticated");
            }
        });
    }
    
    private static void authenticateUser(Activity activity, OktaDomainBlockingBridge bridge) {
        OktaAuthenticationManager oktaManager = bridge.getOktaManager();
        oktaManager.signIn(activity, new OktaAuthenticationManager.AuthenticationCallback() {
            @Override
            public void onSuccess(com.okta.oidc.Tokens tokens) {
                Log.d(TAG, "User authentication successful");
                fetchDomainsWhenAuthenticated(bridge);
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "User authentication failed: " + error);
                handleAuthenticationError(error);
            }
            
            @Override
            public void onCancel() {
                Log.d(TAG, "User cancelled authentication");
            }
        });
    }
    
    private static void fetchAllowedApplications(OktaDomainBlockingBridge bridge) {
        bridge.fetchCustomArrayFromOkta("allowed_applications", new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> applications) {
                Log.d(TAG, "Fetched " + applications.size() + " allowed applications");
                // Handle allowed applications list
                for (String app : applications) {
                    Log.d(TAG, "  Allowed app: " + app);
                }
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error fetching allowed applications: " + error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required for allowed applications");
            }
        });
    }
    
    private static void fetchSecurityPolicies(OktaDomainBlockingBridge bridge) {
        bridge.fetchCustomArrayFromOkta("security_policies", new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> policies) {
                Log.d(TAG, "Fetched " + policies.size() + " security policies");
                // Handle security policies
                for (String policy : policies) {
                    Log.d(TAG, "  Security policy: " + policy);
                }
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error fetching security policies: " + error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required for security policies");
            }
        });
    }
    
    private static void fetchUserGroups(OktaDomainBlockingBridge bridge) {
        bridge.fetchCustomArrayFromOkta("groups", new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> groups) {
                Log.d(TAG, "Fetched " + groups.size() + " user groups");
                // Handle user groups
                for (String group : groups) {
                    Log.d(TAG, "  User group: " + group);
                }
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error fetching user groups: " + error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required for user groups");
            }
        });
    }
    
    private static void fetchDepartmentRestrictions(OktaDomainBlockingBridge bridge) {
        bridge.fetchCustomArrayFromOkta("department_restrictions", new OktaDomainBlockingBridge.DomainBlockingCallback() {
            @Override
            public void onSuccess(List<String> restrictions) {
                Log.d(TAG, "Fetched " + restrictions.size() + " department restrictions");
                // Handle department restrictions
                for (String restriction : restrictions) {
                    Log.d(TAG, "  Department restriction: " + restriction);
                }
            }
            
            @Override
            public void onError(String error) {
                Log.e(TAG, "Error fetching department restrictions: " + error);
            }
            
            @Override
            public void onAuthenticationRequired() {
                Log.w(TAG, "Authentication required for department restrictions");
            }
        });
    }
    
    private static void handleBlockedDomains(List<String> domains) {
        Log.d(TAG, "Handling " + domains.size() + " blocked domains");
        // Implement your domain blocking logic here
        // For example:
        // - Update your web content filter
        // - Notify other components about policy changes
        // - Log policy enforcement events
    }
    
    private static void handleBlockedDomainsError(String error) {
        Log.e(TAG, "Handling blocked domains error: " + error);
        // Implement error handling logic here
        // For example:
        // - Use cached/default policy
        // - Notify user about policy sync failure
        // - Schedule retry
    }
    
    private static void handleAuthenticationError(String error) {
        Log.e(TAG, "Handling authentication error: " + error);
        // Implement authentication error handling here
        // For example:
        // - Show user-friendly error message
        // - Provide retry option
        // - Fall back to local authentication
    }
    
    private static void notifyDomainPolicyUpdated(List<String> domains) {
        Log.d(TAG, "Notifying that domain policy was updated with " + domains.size() + " domains");
        // Implement policy update notification here
        // For example:
        // - Broadcast intent to other components
        // - Update UI to reflect new policy
        // - Log policy change event
    }
} 