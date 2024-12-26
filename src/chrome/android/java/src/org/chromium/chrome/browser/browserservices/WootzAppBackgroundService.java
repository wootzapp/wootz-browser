package org.chromium.chrome.browser.browserservices;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;
import androidx.core.app.NotificationCompat;
import org.chromium.base.ContextUtils;
// import org.chromium.chrome.R;
import org.chromium.components.browser_ui.notifications.ForegroundServiceUtils;
import org.json.JSONArray;
import org.json.JSONObject;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Timer;
import java.util.TimerTask;
import android.app.PendingIntent;
import android.Manifest;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings; 
import android.net.Uri;     

public class WootzAppBackgroundService extends Service {
    private static final String TAG = "WootzService";
    private static final String WOOTZ_JOBS_KEY = "Chrome.Wootzapp.Jobs";
    private static final String WOOTZ_RESULTS_KEY = "Chrome.Wootzapp.JobsResult";
    private static final long FETCH_INTERVAL_MS = 15000; // 15 seconds
    private static final int NOTIFICATION_ID = 1001;
    private static final String CHANNEL_ID = "wootz_service_channel";
    private Timer timer;
    private boolean isRunning;
    private static final String[] REQUIRED_PERMISSIONS = {
        Manifest.permission.FOREGROUND_SERVICE,
        Manifest.permission.POST_NOTIFICATIONS
    };
    @Override
    public void onCreate() {
        super.onCreate();
        isRunning = true;
        createNotificationChannel();
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        requestIgnoreBatteryOptimizations();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "Background Service",
                NotificationManager.IMPORTANCE_LOW
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }

        // For Android 12 (API 31) and above
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            try {
                // Request foreground service permission if needed
                if (checkSelfPermission(Manifest.permission.FOREGROUND_SERVICE) 
                        != PackageManager.PERMISSION_GRANTED) {
                    return START_NOT_STICKY;
                }
                
                // Create notification with proper flags for Android 12+
                Notification.Builder builder = new Notification.Builder(this, CHANNEL_ID)
                    .setContentTitle("Service Active")
                    .setContentText("Running in background")
                    .setSmallIcon(android.R.drawable.ic_dialog_info)
                    .setForegroundServiceBehavior(Notification.FOREGROUND_SERVICE_IMMEDIATE);

                startForeground(NOTIFICATION_ID, builder.build());
            } catch (Exception e) {
                Log.e(TAG, "Error starting foreground service on Android 12+", e);
                return START_NOT_STICKY;
            }
        } else {
            // For Android 11 and below
            Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Service Active")
                .setContentText("Running in background")
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setOngoing(true)
                .build();

            startForeground(NOTIFICATION_ID, notification);
        }

        return START_STICKY;
    }

    private void requestIgnoreBatteryOptimizations() {
        Intent batteryIntent = new Intent();
        batteryIntent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
        batteryIntent.setData(Uri.parse("package:" + getPackageName()));
        batteryIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(batteryIntent);
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID,
                    "Wootz Background Service",
                    NotificationManager.IMPORTANCE_LOW);
            channel.setDescription("Background service for Wootz App");
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            if (notificationManager != null) {
                notificationManager.createNotificationChannel(channel);
            }
        }
    }
    private Notification createNotification() {
        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Wootzapp background service")
                .setContentText("Background service is running")
                // .setSmallIcon(R.drawable.ic_chrome)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .setOngoing(true)
                .build();
    }
    @Override
    public void onDestroy() {
        try {
            // Save state before destruction
            saveCurrentState();
            
            // Ensure we're still in foreground mode
            startForeground(NOTIFICATION_ID, createNotification());
            
            // Try to restart service
            Intent restartService = new Intent(getApplicationContext(), this.getClass());
            startService(restartService);
            
        } catch (Exception e) {
            Log.e(TAG, "Error in onDestroy", e);
        }
        super.onDestroy();
    }
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    private void startJobProcessing() {
        Log.d(TAG, "Job processing started");
        timer = new Timer();
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (!isRunning) return;
                processJobs();
            }
        }, 0, FETCH_INTERVAL_MS);
    }
    private void processJobs() {
        Log.d(TAG, "Processing jobs...");
        SharedPreferences prefs = ContextUtils.getAppSharedPreferences();
        String jobsJson = prefs.getString(WOOTZ_JOBS_KEY, "[]");
        String resultsJson = prefs.getString(WOOTZ_RESULTS_KEY, "[]");
        try {
            JSONArray jobs = new JSONArray(jobsJson);
            JSONArray results = new JSONArray(resultsJson);
            // Limit results size (keep last 100 results)
            while (results.length() > 100) {
                results.remove(0);
            }
            
            // Process each URL in jobs list
            for (int i = 0; i < jobs.length(); i++) {
                String url = jobs.getString(i);
                Log.d(TAG, "Fetching URL: " + url);
                String response = fetchUrl(url);
                
                JSONObject result = new JSONObject();
                result.put("url", url);
                result.put("timestamp", System.currentTimeMillis());
                result.put("response", response);
                
                results.put(result);
            }
            // Save results
            prefs.edit()
                .putString(WOOTZ_RESULTS_KEY, results.toString())
                .apply();
            Log.d(TAG, "Results saved successfully");
        } catch (Exception e) {
            Log.e(TAG, "Error processing jobs", e);
        }
    }
    private String fetchUrl(String urlString) {
        Log.d(TAG, "Fetching URL: " + urlString);
        try {
            URL url = new URL(urlString);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(10000);
            conn.setReadTimeout(10000);
            StringBuilder response = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(conn.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    response.append(line);
                }
            }
            Log.d(TAG, "Fetch successful for URL: " + urlString);
            return response.toString();
        } catch (Exception e) {
            Log.e(TAG, "Error fetching URL: " + urlString, e);
            return "Error: " + e.getMessage();
        }
    }
    @Override
    public void onTaskRemoved(Intent rootIntent) {
        try {
            // Keep the service alive
            startForeground(NOTIFICATION_ID, createNotification());
            
            // Save state before potential restart
            saveCurrentState();
            
            // Create restart intent with proper flags
            Intent restartServiceIntent = new Intent(getApplicationContext(), this.getClass());
            restartServiceIntent.setPackage(getPackageName());
            
            // Use FLAG_IMMUTABLE for Android 12+
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? 
                PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_ONE_SHOT : 
                PendingIntent.FLAG_ONE_SHOT;
                
            PendingIntent restartServicePendingIntent = PendingIntent.getService(
                getApplicationContext(), 1, restartServiceIntent, flags);
            
            // Ensure service stays alive
            startService(restartServiceIntent);
            
        } catch (Exception e) {
            Log.e(TAG, "Error in onTaskRemoved", e);
        }
        super.onTaskRemoved(rootIntent);
    }

    private void saveCurrentState() {
        try {
            SharedPreferences prefs = ContextUtils.getAppSharedPreferences();
            SharedPreferences.Editor editor = prefs.edit();
            
            // Save any important state atomically
            editor.putLong("last_sync_time", System.currentTimeMillis())
                  .putBoolean("service_running", true)
                  .apply();
                  
        } catch (Exception e) {
            Log.e(TAG, "Error saving state", e);
        }
    }
}