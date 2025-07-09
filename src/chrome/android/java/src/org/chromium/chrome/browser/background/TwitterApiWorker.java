package org.chromium.chrome.browser.background;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.work.Worker;
import androidx.work.WorkerParameters;

import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;

public class TwitterApiWorker extends Worker {
    private static final String TAG = "TwitterApiWorker";

    private TwitterScrapingBridgeJni mWorkerBridge;

    public TwitterApiWorker(@NonNull Context context, @NonNull WorkerParameters params) {
        super(context, params);
    }
    
    @Override
    public Result doWork() {
        Log.i(TAG, "Starting Twitter API periodic task");
        try {
            // Ensure native libraries are loaded before calling JNI methods
            if (!LibraryLoader.getInstance().isInitialized()) {
                Log.i(TAG, "Loading native libraries...");
                LibraryLoader.getInstance().ensureInitialized();
            }

            // Initialize the bridge when the worker is created
            mWorkerBridge = TwitterScrapingBridgeJni.getInstance();

            if (mWorkerBridge == null) {
                Log.e(TAG, "Failed to initialize TwitterScrapingBridgeJni");
                return Result.failure();
            }
            
            mWorkerBridge.makeTwitterApiCall();
                        
            Log.i(TAG, "Twitter API calls initiated successfully");
            return Result.success();
        } catch (Exception e) {
            Log.e(TAG, "Failed to execute Twitter API calls: " + e.getMessage(), e);
            return Result.retry();
        }
    }
}