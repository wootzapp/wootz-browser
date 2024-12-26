package org.chromium.chrome.browser.extensions;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.os.Build;
import androidx.core.app.NotificationCompat;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.ChromeTabbedActivity;

public class BrowsingHistoryService extends Service {
    private static final String TAG = "BrowsingHistoryService";
    private static final long INTERVAL = 30000; // 30 seconds in milliseconds
    private static final int NOTIFICATION_ID = 1;
    private static final String CHANNEL_ID = "BrowsingHistoryChannel";

    private Handler handler = new Handler();
    private Runnable historyRunnable = new Runnable() {
        @Override
        public void run() {
            pingHistory();
            handler.postDelayed(this, INTERVAL);
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(NOTIFICATION_ID, createNotification());
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Service started");
        handler.post(historyRunnable);
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void pingHistory() {
        Log.d(TAG, "Pinging history every 30 seconds");
        // Add your history tracking logic here
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(historyRunnable);
        Log.d(TAG, "Service destroyed");
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID,
                    "Browsing History Service Channel",
                    NotificationManager.IMPORTANCE_LOW
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(serviceChannel);
        }
    }

    private Notification createNotification() {
        Intent notificationIntent = new Intent(this, ChromeTabbedActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                this,
                0,
                notificationIntent,
                PendingIntent.FLAG_IMMUTABLE
        );

        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Browsing History Service")
                .setContentText("Tracking browsing history")
                .setSmallIcon(R.drawable.ic_chrome)
                .setContentIntent(pendingIntent)
                .build();
    }
}
