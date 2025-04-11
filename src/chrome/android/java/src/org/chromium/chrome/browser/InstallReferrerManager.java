package org.chromium.chrome.browser;

import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

import com.android.installreferrer.api.InstallReferrerClient;
import com.android.installreferrer.api.InstallReferrerStateListener;
import com.android.installreferrer.api.ReferrerDetails;

public class InstallReferrerManager implements InstallReferrerStateListener {
    private static final String TAG = "InstallReferrerManager";
    private InstallReferrerClient referrerClient;
    private Context context;

    public InstallReferrerManager(Context context) {
        this.context = context;
    }

    public void start() {
        referrerClient = InstallReferrerClient.newBuilder(context).build();
        referrerClient.startConnection(this);
    }

    @Override
    public void onInstallReferrerSetupFinished(int responseCode) {
        switch (responseCode) {
            case InstallReferrerClient.InstallReferrerResponse.OK:
                try {
                    Log.d(TAG, "Install referrer connection established");
                    ReferrerDetails response = referrerClient.getInstallReferrer();
                    
                    // Get and process referrer
                    String referrerUrl = response.getInstallReferrer();
                    long referrerClickTime = response.getReferrerClickTimestampSeconds();
                    long appInstallTime = response.getInstallBeginTimestampSeconds();
                    boolean instantExperienceLaunched = response.getGooglePlayInstantParam();
                    
                    Log.d(TAG, "Referrer URL: " + referrerUrl);
                    
                    // Process the referrer data - important for Branch attribution
                    processReferrerData(referrerUrl, referrerClickTime, appInstallTime);
                    
                    // Close the connection
                    referrerClient.endConnection();
                } catch (RemoteException e) {
                    Log.e(TAG, "Error getting install referrer: " + e.getMessage());
                }
                break;
            case InstallReferrerClient.InstallReferrerResponse.FEATURE_NOT_SUPPORTED:
                Log.w(TAG, "Install referrer not supported on this device");
                break;
            case InstallReferrerClient.InstallReferrerResponse.SERVICE_UNAVAILABLE:
                Log.w(TAG, "Install referrer service unavailable");
                break;
            case InstallReferrerClient.InstallReferrerResponse.SERVICE_DISCONNECTED:
                Log.w(TAG, "Install referrer service disconnected");
                break;
            case InstallReferrerClient.InstallReferrerResponse.DEVELOPER_ERROR:
                Log.e(TAG, "Install referrer error: developer error");
                break;
        }
    }

    @Override
    public void onInstallReferrerServiceDisconnected() {
        Log.w(TAG, "Install referrer service disconnected");
    }
    
    private void processReferrerData(String referrerUrl, long clickTime, long installTime) {
        // If using Branch, you don't need to manually process this
        // Branch SDK will automatically collect the referrer when initialized
        // But you can use this data for your own analytics as well
        
        // Example of parsing UTM parameters from the referrer URL
        if (referrerUrl != null && !referrerUrl.isEmpty()) {
            // Referrer format: utm_source=source&utm_medium=medium&utm_campaign=campaign
            String[] params = referrerUrl.split("&");
            for (String param : params) {
                String[] keyValue = param.split("=");
                if (keyValue.length == 2) {
                    String key = keyValue[0];
                    String value = keyValue[1];
                    Log.d(TAG, "Referrer param: " + key + " = " + value);
                    
                    // Store these values if needed
                }
            }
        }
    }
}