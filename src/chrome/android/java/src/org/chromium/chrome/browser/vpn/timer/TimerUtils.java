package org.chromium.chrome.browser.vpn.timer;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import org.chromium.base.IntentUtils;

import java.util.Calendar;
import java.util.Date;

public class TimerUtils {
    private static int sVpnActionRequestCode = 1001;

    public static void scheduleVpnAction(Context context, int minutes) {
        Intent vpnActionIntent = new Intent(context, TimerVpnActionReceiver.class);
        PendingIntent pendingIntent =
                PendingIntent.getBroadcast(
                        context,
                        sVpnActionRequestCode,
                        vpnActionIntent,
                        0 | IntentUtils.getPendingIntentMutabilityFlag(true));
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis());
        calendar.add(Calendar.MINUTE, minutes);

        Date date = calendar.getTime();
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        assert alarmManager != null;
        alarmManager.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, date.getTime(), pendingIntent);
    }

    public static void cancelScheduledVpnAction(Context context) {
        Intent vpnActionIntent = new Intent(context, TimerVpnActionReceiver.class);
        PendingIntent pendingIntent =
                PendingIntent.getBroadcast(
                        context,
                        sVpnActionRequestCode,
                        vpnActionIntent,
                        0 | IntentUtils.getPendingIntentMutabilityFlag(true));

        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        assert alarmManager != null;
        alarmManager.cancel(pendingIntent);
    }
}