package org.chromium.chrome.browser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import com.android.installreferrer.api.InstallReferrerClient;
import com.android.installreferrer.api.InstallReferrerStateListener;
import com.android.installreferrer.api.ReferrerDetails;

import org.chromium.base.ContextUtils;

public class InstallReferrerManager implements InstallReferrerStateListener {
    private static final String TAG = "InstallReferrerManager";
    private static final String REFERRER_PROCESSED_KEY = "install_referrer_processed";
    
    private InstallReferrerClient referrerClient;
    private Context context;
    private int retryCount = 0;
    private static final int MAX_RETRIES = 3;

    public InstallReferrerManager(Context context) {
        this.context = context;
    }

    public void start() {
        // Check if we've already processed the referrer
        SharedPreferences prefs = ContextUtils.getAppSharedPreferences();
        boolean alreadyProcessed = prefs.getBoolean(REFERRER_PROCESSED_KEY, false);
        
        if (alreadyProcessed) {
            Log.e(TAG, "Install referrer already processed, skipping");
            return;
        }
        
        Log.e(TAG, "Starting install referrer connection");
        referrerClient = InstallReferrerClient.newBuilder(context).build();
        referrerClient.startConnection(this);
    }

    @Override
    public void onInstallReferrerSetupFinished(int responseCode) {
        switch (responseCode) {
            case InstallReferrerClient.InstallReferrerResponse.OK:
                try {
                    Log.e(TAG, "Install referrer connection established");
                    ReferrerDetails response = referrerClient.getInstallReferrer();
                    
                    // Get and process referrer
                    String referrerUrl = response.getInstallReferrer();
                    long referrerClickTime = response.getReferrerClickTimestampSeconds();
                    long appInstallTime = response.getInstallBeginTimestampSeconds();
                    boolean instantExperienceLaunched = response.getGooglePlayInstantParam();
                    
                    Log.e(TAG, "Referrer URL: " + referrerUrl);
                    
                    // Process the referrer data
                    if (referrerUrl != null && !referrerUrl.isEmpty()) {
                        
                        // Check for Branch link in the referrer
                        if (referrerUrl.contains("branch.wootz.app") || 
                            referrerUrl.contains("branch_link=") || 
                            referrerUrl.contains("link=")) {
                            Log.e(TAG, "FOUND BRANCH LINK in referrer: " + referrerUrl);
                        }
                        
                        // 1. Store the referrer
                        SharedPreferences prefs = ContextUtils.getAppSharedPreferences();
                        prefs.edit()
                            .putString("install_referrer", referrerUrl)
                            .putLong("referrer_click_time", referrerClickTime)
                            .putLong("app_install_time", appInstallTime)
                            .putBoolean(REFERRER_PROCESSED_KEY, true)
                            .apply();
                        
                        // 2. Forward to Branch SDK using the most basic approach
                        try {
                            // Store the referrer in a place Branch can find it
                            android.content.Context appContext = context.getApplicationContext();
                            android.content.SharedPreferences prefsBranch = 
                                appContext.getSharedPreferences("io.branch.referral.InstallReferrer", 
                                                               Context.MODE_PRIVATE);
                            
                            // Log what was previously stored
                            String previousReferrer = prefsBranch.getString("install_referrer", "");
                            Log.e(TAG, "Previous Branch referrer: " + previousReferrer);
                            
                            // Store the new referrer
                            prefsBranch.edit().putString("install_referrer", referrerUrl).apply();
                            
                            Log.e(TAG, "Stored install referrer for Branch SDK");
                            
                            // Check what's now stored
                            String storedReferrer = prefsBranch.getString("install_referrer", "");
                            Log.e(TAG, "Verified stored Branch referrer: " + storedReferrer);
                            
                            // 3. Force Branch to check for the referrer
                            new Handler(Looper.getMainLooper()).post(() -> {
                                try {
                                    // Get a new session - this should trigger Branch to read the referrer
                                    Log.e(TAG, "About to request latest Branch parameters");
                                    io.branch.referral.Branch.getInstance().getLatestReferringParams();
                                    Log.e(TAG, "Requested latest Branch parameters");
                                    
                                    // Try to log the current Branch session params
                                    try {
                                        org.json.JSONObject params = io.branch.referral.Branch.getInstance().getLatestReferringParams();
                                        Log.e(TAG, "Current Branch session params: " + params.toString());
                                        boolean clickedBranchLink = params.optBoolean("+clicked_branch_link", false);
                                        Log.e(TAG, "Branch reports clicked_branch_link: " + clickedBranchLink);
                                    } catch (Exception e) {
                                        Log.e(TAG, "Error getting Branch session params: " + e.getMessage());
                                    }
                                } catch (Exception e) {
                                    Log.e(TAG, "Error getting Branch parameters: " + e.getMessage());
                                }
                            });
                        } catch (Exception e) {
                            Log.e(TAG, "Error forwarding referrer to Branch: " + e.getMessage());
                        }
                        
                        // 4. Parse UTM parameters
                        parseUtmParameters(referrerUrl);
                    }
                    
                    // Close the connection
                    referrerClient.endConnection();
                } catch (RemoteException e) {
                    Log.e(TAG, "Error getting install referrer: " + e.getMessage());
                    retryConnection();
                }
                break;
            case InstallReferrerClient.InstallReferrerResponse.FEATURE_NOT_SUPPORTED:
                Log.e(TAG, "Install referrer not supported on this device");
                break;
            case InstallReferrerClient.InstallReferrerResponse.SERVICE_UNAVAILABLE:
                Log.e(TAG, "Install referrer service unavailable");
                retryConnection();
                break;
            case InstallReferrerClient.InstallReferrerResponse.SERVICE_DISCONNECTED:
                Log.e(TAG, "Install referrer service disconnected");
                retryConnection();
                break;
            case InstallReferrerClient.InstallReferrerResponse.DEVELOPER_ERROR:
                Log.e(TAG, "Install referrer error: developer error");
                break;
        }
    }

    @Override
    public void onInstallReferrerServiceDisconnected() {
        Log.e(TAG, "Install referrer service disconnected");
        retryConnection();
    }
    
    private void retryConnection() {
        if (retryCount < MAX_RETRIES) {
            retryCount++;
            Log.e(TAG, "Retrying referrer connection, attempt " + retryCount);
            new Handler(Looper.getMainLooper()).postDelayed(() -> {
                try {
                    referrerClient.startConnection(this);
                } catch (Exception e) {
                    Log.e(TAG, "Error retrying referrer connection", e);
                }
            }, 2000); // 2 second delay
        } else {
            Log.e(TAG, "Max retries reached for referrer connection");
        }
    }
    
    private void parseUtmParameters(String referrerUrl) {
        // Example of parsing UTM parameters from the referrer URL
        if (referrerUrl != null && !referrerUrl.isEmpty()) {
            // Referrer format: utm_source=source&utm_medium=medium&utm_campaign=campaign
            String[] params = referrerUrl.split("&");
            for (String param : params) {
                String[] keyValue = param.split("=");
                if (keyValue.length == 2) {
                    String key = keyValue[0];
                    String value = keyValue[1];
                    Log.e(TAG, "Referrer param: " + key + " = " + value);
                    
                    // Store UTM source if found
                    if (key.equals("utm_source") && IntentHandler.class != null) {
                        try {
                            IntentHandler.storeUtmSource(value);
                            Log.e(TAG, "Stored UTM source: " + value);
                        } catch (Exception e) {
                            Log.e(TAG, "Error storing UTM source", e);
                        }
                    }
                }
            }
        }
    }
}