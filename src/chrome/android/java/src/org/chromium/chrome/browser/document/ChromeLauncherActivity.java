// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.document;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import io.branch.referral.Branch;
import io.branch.referral.BranchError;
import org.json.JSONObject;

import com.google.android.material.color.DynamicColors;


import org.chromium.base.TraceEvent;
import org.chromium.chrome.browser.LaunchIntentDispatcher;
import org.chromium.chrome.browser.IntentHandler;
import android.text.TextUtils;
import android.content.Intent;
import org.chromium.chrome.browser.ChromeTabbedActivity;
import org.chromium.content_public.browser.LoadUrlParams;

/**
 * Dispatches incoming intents to the appropriate activity based on the current configuration and
 * Intent fired.
 */
public class ChromeLauncherActivity extends Activity {
    public static String ext_utm_source = "";
    private boolean isFirstRun = true;
    private static final String TAG = "ChromeLauncherActivity";
    @Override
    public void onCreate(Bundle savedInstanceState) {
        // Third-party code adds disk access to Activity.onCreate. http://crbug.com/619824
        TraceEvent.begin("ChromeLauncherActivity.onCreate");
        super.onCreate(savedInstanceState);

        // TODO(crbug.com/40775606): Figure out a scalable way to apply overlays to
        // activities like this.
        applyThemeOverlays();

        @LaunchIntentDispatcher.Action
        int dispatchAction = LaunchIntentDispatcher.dispatch(this, getIntent());
        switch (dispatchAction) {
            case LaunchIntentDispatcher.Action.FINISH_ACTIVITY:
                finish();
                break;
            case LaunchIntentDispatcher.Action.FINISH_ACTIVITY_REMOVE_TASK:
                this.finishAndRemoveTask();
                break;
            default:
                assert false
                        : "Intent dispatcher finished with action "
                                + dispatchAction
                                + ", finishing anyway";
                finish();
                break;
        }
        TraceEvent.end("ChromeLauncherActivity.onCreate");
    }

    private void applyThemeOverlays() {
        DynamicColors.applyToActivityIfAvailable(this);
    }

    @Override
    protected void onStart() {
        super.onStart();
        
        if (getIntent() != null) {
            getIntent().putExtra("branch_force_new_session", true);
        }

        // Get the latest Branch deep link data in onStart
        Branch.sessionBuilder(this)
            .withCallback(new Branch.BranchReferralInitListener() {
                @Override
                public void onInitFinished(JSONObject referringParams, BranchError error) {
                    if (error == null && referringParams != null) {
                        Log.e(TAG, "onStart: Branch session data: " + referringParams.toString());
                        
                        // Store the entire Branch data JSON in SharedPreferences
                        try {
                            android.content.SharedPreferences prefs = getSharedPreferences("branch_data", MODE_PRIVATE);
                            android.content.SharedPreferences.Editor editor = prefs.edit();
                            
                            // Store the full JSON for reference
                            editor.putString("branch_data_json", referringParams.toString());
                            
                            // Store individual parameters
                            if (referringParams.has("~channel")) {
                                String utmSource = referringParams.optString("~channel", "");
                                editor.putString("utm_source_wootzapp", utmSource);
                                Log.e(TAG, "Stored utm_source_wootzapp: " + utmSource);
                            }
                            
                            if (referringParams.has("~campaign")) {
                                String utmCampaign = referringParams.optString("~campaign", "");
                                editor.putString("utm_campaign_wootzapp", utmCampaign);
                                Log.e(TAG, "Stored utm_campaign_wootzapp: " + utmCampaign);
                            }
                            
                            if (referringParams.has("~feature")) {
                                String utmMedium = referringParams.optString("~feature", "");
                                editor.putString("utm_medium_wootzapp", utmMedium);
                                Log.e(TAG, "Stored utm_medium_wootzapp: " + utmMedium);
                            }
                            
                            if (referringParams.has("$canonical_url")) {
                                String url = referringParams.optString("$canonical_url", "");
                                editor.putString("branch_canonical_url_wootzapp", url);
                                Log.e(TAG, "Stored branch_canonical_url_wootzapp: " + url);
                            }
                            
                            // Commit the changes
                            editor.apply();
                            Log.e(TAG, "Branch data stored in SharedPreferences");
                            
                            // Also update IntentHandler if needed
                            String utmSource = referringParams.optString("~channel", "");
                            if (!TextUtils.isEmpty(utmSource)) {
                                IntentHandler.ext_utm_source = utmSource;
                                IntentHandler.switchIconBasedOnUtm(utmSource);
                                IntentHandler.storeUtmSource(utmSource);
                                IntentHandler.processStoredUtmSourceIfNeeded();
                            }
                        } catch (Exception e) {
                            Log.e(TAG, "Error storing Branch data in SharedPreferences", e);
                        }
                    } else if (error != null) {
                        Log.e(TAG, "onStart: Branch initialization error: " + error.getMessage());
                    }
                }
            })
            .withData(getIntent().getData())
            .init();
    }
}
