// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.safety_hub;

import android.content.Context;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.password_manager.PasswordCheckReferrer;
import org.chromium.chrome.browser.password_manager.PasswordManagerHelper;
import org.chromium.chrome.browser.password_manager.PasswordManagerUtilBridge;
import org.chromium.chrome.browser.preferences.Pref;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.chrome.browser.sync.SyncServiceFactory;
import org.chromium.components.background_task_scheduler.BackgroundTaskSchedulerFactory;
import org.chromium.components.background_task_scheduler.NativeBackgroundTask;
import org.chromium.components.background_task_scheduler.TaskIds;
import org.chromium.components.background_task_scheduler.TaskInfo;
import org.chromium.components.background_task_scheduler.TaskParameters;
import org.chromium.components.prefs.PrefService;
import org.chromium.components.signin.base.CoreAccountInfo;
import org.chromium.components.sync.SyncService;
import org.chromium.components.user_prefs.UserPrefs;

import java.util.concurrent.TimeUnit;

/** Manages the scheduling of Safety Hub fetch jobs. */
public class SafetyHubFetchService extends NativeBackgroundTask {
    private static final int SAFETY_HUB_JOB_INTERVAL_IN_DAYS = 1;

    /** See {@link ChromeActivitySessionTracker#onForegroundSessionStart()}. */
    public static void onForegroundSessionStart() {
        if (ChromeFeatureList.isEnabled(ChromeFeatureList.SAFETY_HUB)) {
            scheduleFetchJobAfterDelay(0);
        } else {
            cancelFetchJob();
        }
    }

    /** Schedules the fetch job to run after the given delay if it doesn't already exist. */
    private static void scheduleFetchJobAfterDelay(long delayMs) {
        TaskInfo.TimingInfo oneOffTimingInfo =
                TaskInfo.OneOffInfo.create()
                        .setWindowStartTimeMs(delayMs)
                        .setWindowEndTimeMs(delayMs)
                        .build();

        TaskInfo taskInfo =
                TaskInfo.createTask(TaskIds.SAFETY_HUB_JOB_ID, oneOffTimingInfo)
                        .setUpdateCurrent(false)
                        .setIsPersisted(true)
                        .build();

        BackgroundTaskSchedulerFactory.getScheduler()
                .schedule(ContextUtils.getApplicationContext(), taskInfo);
    }

    private static void cancelFetchJob() {
        BackgroundTaskSchedulerFactory.getScheduler()
                .cancel(ContextUtils.getApplicationContext(), TaskIds.SAFETY_HUB_JOB_ID);
    }

    /** Schedules the next fetch job to run after a delay. */
    private void scheduleNextFetchJob() {
        long nextFetchDelayMs = TimeUnit.DAYS.toMillis(SAFETY_HUB_JOB_INTERVAL_IN_DAYS);

        // Cancel existing job if it wasn't already stopped.
        cancelFetchJob();
        scheduleFetchJobAfterDelay(nextFetchDelayMs);
    }

    private static boolean checkConditions() {
        Profile profile = ProfileManager.getLastUsedRegularProfile();
        PasswordManagerHelper passwordManagerHelper = PasswordManagerHelper.getForProfile(profile);
        SyncService syncService = SyncServiceFactory.getForProfile(profile);
        String accountEmail =
                (syncService != null)
                        ? CoreAccountInfo.getEmailFrom(syncService.getAccountInfo())
                        : null;

        return PasswordManagerHelper.hasChosenToSyncPasswords(syncService)
                && PasswordManagerUtilBridge.areMinUpmRequirementsMet()
                && passwordManagerHelper.canUseUpm()
                && accountEmail != null;
    }

    @Override
    protected int onStartTaskBeforeNativeLoaded(
            Context context, TaskParameters taskParameters, TaskFinishedCallback callback) {
        return StartBeforeNativeResult.LOAD_NATIVE;
    }

    @Override
    protected void onStartTaskWithNative(
            Context context, TaskParameters taskParameters, TaskFinishedCallback callback) {
        fetchBreachedCredentialsCount(callback);
    }

    @Override
    protected boolean onStopTaskBeforeNativeLoaded(Context context, TaskParameters taskParameters) {
        // Reschedule task if native didn't complete loading, the call to GMSCore wouldn't have been
        // made at this point.
        return true;
    }

    @Override
    protected boolean onStopTaskWithNative(Context context, TaskParameters taskParameters) {
        // GMSCore has no mechanism to abort dispatched tasks.
        return false;
    }

    private void fetchBreachedCredentialsCount(final TaskFinishedCallback callback) {
        if (!checkConditions()) {
            // TODO(324562205): Listen to sync status changes and schedule/cancel tasks accordingly,
            // instead of rescheduling when conditions are not met.
            callback.taskFinished(/* needsReschedule= */ true);
            return;
        }

        Profile profile = ProfileManager.getLastUsedRegularProfile();
        PasswordManagerHelper passwordManagerHelper = PasswordManagerHelper.getForProfile(profile);
        PrefService prefService = UserPrefs.get(profile);
        SyncService syncService = SyncServiceFactory.getForProfile(profile);

        assert syncService != null;
        String accountEmail = CoreAccountInfo.getEmailFrom(syncService.getAccountInfo());

        passwordManagerHelper.getBreachedCredentialsCount(
                PasswordCheckReferrer.SAFETY_CHECK,
                accountEmail,
                count -> {
                    // TODO(b/324562205): Find another way to store the data tied to the signed in
                    // account or clear the pref when the user signs out.
                    prefService.setInteger(Pref.BREACHED_CREDENTIALS_COUNT, count);
                    callback.taskFinished(/* needsReschedule= */ false);
                    scheduleNextFetchJob();
                },
                error -> {
                    callback.taskFinished(/* needsReschedule= */ true);
                });
    }
}
