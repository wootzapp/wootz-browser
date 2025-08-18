// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.document;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.google.android.material.color.DynamicColors;

import org.chromium.chrome.browser.firstrun.FirstRunActivity;
import org.chromium.chrome.browser.firstrun.FirstRunStatus;
import org.chromium.base.TraceEvent;
import org.chromium.chrome.browser.LaunchIntentDispatcher;

/**
 * Dispatches incoming intents to the appropriate activity based on the current configuration and
 * Intent fired.
 */
public class ChromeLauncherActivity extends Activity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        // Third-party code adds disk access to Activity.onCreate. http://crbug.com/619824
        TraceEvent.begin("ChromeLauncherActivity.onCreate");
        super.onCreate(savedInstanceState);
        Log.e("ChromeLauncherActivity", "onCreate");

        // Skip welcome page
        FirstRunStatus.setSkipWelcomePage(true);

        // Handle Branch intents by redirecting to first run experience
        if (getIntent() != null && getIntent().getData() != null 
                && "branch.wootz.app".equals(getIntent().getData().getHost())) {
            // Instead of just finishing, redirect to first run
            redirectBranchIntentToFirstRun();
            TraceEvent.end("ChromeLauncherActivity.onCreate");
            Log.e("ChromeLauncherActivity", "redirectBranchIntentToFirstRun");
            return;
        }

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
    private void redirectBranchIntentToFirstRun() {
        // Import the FirstRunActivity class at the top of the file
        android.content.Intent firstRunIntent = new android.content.Intent(
                this, org.chromium.chrome.browser.firstrun.FirstRunActivity.class);
        // Pass along the original intent data
        firstRunIntent.setData(getIntent().getData());
        // Copy any extras from the original intent
        if (getIntent().getExtras() != null) {
            firstRunIntent.putExtras(getIntent().getExtras());
        }
        startActivity(firstRunIntent);
        finish();
    }
}
