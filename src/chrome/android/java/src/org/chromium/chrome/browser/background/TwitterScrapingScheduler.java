package org.chromium.chrome.browser.background;

import android.content.Context;
import android.util.Log;

import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.PeriodicWorkRequest;
import androidx.work.WorkManager;

import java.util.concurrent.TimeUnit;

public class TwitterScrapingScheduler {
    private static final String TAG = "TwitterScrapingScheduler";
    private static final String WORK_NAME = "twitter_scraping_periodic_work";
    private static final long REPEAT_INTERVAL_MINUTES = 15;

    /**
     * Schedules the periodic Twitter API fetch task
     * @param context Application context
     */
    public static void scheduleTwitterScrapingTask(Context context) {
        Log.i(TAG, "Scheduling Twitter Scraping periodic task");
        
        try {
            // Create constraints for the work
            Constraints constraints = new Constraints.Builder()
                    .setRequiredNetworkType(NetworkType.CONNECTED)
                    .setRequiresBatteryNotLow(false)
                    .setRequiresCharging(false)
                    .setRequiresDeviceIdle(false)
                    .setRequiresStorageNotLow(false)
                    .build();

            // Create the periodic work request
            PeriodicWorkRequest twitterScrapingWork = new PeriodicWorkRequest.Builder(
                    TwitterApiWorker.class,
                    REPEAT_INTERVAL_MINUTES,
                    TimeUnit.MINUTES)
                    .setConstraints(constraints)
                    .addTag("twitter_scraping")
                    .addTag("periodic_background_task")
                    .build();

            // Schedule the work
            WorkManager.getInstance(context).enqueueUniquePeriodicWork(
                    WORK_NAME,
                    ExistingPeriodicWorkPolicy.UPDATE, // Update if already exists
                    twitterScrapingWork);

            Log.i(TAG, "Successfully scheduled Twitter Scraping task to run every " + 
                       REPEAT_INTERVAL_MINUTES + " minutes");
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to schedule Twitter Scraping task: " + e.getMessage(), e);
        }
    }

    /**
     * Cancels the periodic Twitter API fetch task
     * @param context Application context
     */
    public static void cancelTwitterScrapingTask(Context context) {
        Log.i(TAG, "Cancelling Twitter Scraping periodic task");
        
        try {
            WorkManager.getInstance(context).cancelUniqueWork(WORK_NAME);
            Log.i(TAG, "Successfully cancelled Twitter Scraping task");
        } catch (Exception e) {
            Log.e(TAG, "Failed to cancel Twitter Scraping task: " + e.getMessage(), e);
        }
    }

    /**
     * Checks if the Twitter Scraping task is currently scheduled
     * @param context Application context
     */
    public static void checkTaskStatus(Context context) {
        Log.i(TAG, "Checking Twitter Scraping task status");
        
        try {
            WorkManager.getInstance(context)
                    .getWorkInfosForUniqueWork(WORK_NAME)
                    .addListener(() -> {
                        Log.i(TAG, "Work status check completed");
                    }, context.getMainExecutor());
        } catch (Exception e) {
            Log.e(TAG, "Failed to check task status: " + e.getMessage(), e);
        }
    }
}