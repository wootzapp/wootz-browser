package org.chromium.chrome.browser.vpn.utils;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.os.Build;

import androidx.core.content.ContextCompat;

import org.chromium.chrome.browser.vpn.timer.TimerUtils;
import org.chromium.chrome.browser.vpn.wireguard.WireguardService;
import org.chromium.chrome.browser.vpn.wireguard.WireguardUtils;

public class WootzVpnProfileUtils {
    private static volatile WootzVpnProfileUtils sWootzVpnProfileUtils;
    private static Object sMutex = new Object();

    private WootzVpnProfileUtils() {}

    public static WootzVpnProfileUtils getInstance() {
        WootzVpnProfileUtils result = sWootzVpnProfileUtils;
        if (result == null) {
            synchronized (sMutex) {
                result = sWootzVpnProfileUtils;
                if (result == null) sWootzVpnProfileUtils = result = new WootzVpnProfileUtils();
            }
        }
        return result;
    }

    public boolean isWootzVPNConnected(Context context) {
        return WireguardUtils.isServiceRunningInForeground(context, WireguardService.class);
    }

    public boolean isVPNRunning(Context context) {
        ConnectivityManager connectivityManager =
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        boolean isVpnConnected = false;
        if (connectivityManager != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                NetworkCapabilities capabilities = connectivityManager.getNetworkCapabilities(
                        connectivityManager.getActiveNetwork());
                isVpnConnected = capabilities != null
                        ? capabilities.hasTransport(NetworkCapabilities.TRANSPORT_VPN)
                        : false;
            } else {
                NetworkInfo activeNetwork = connectivityManager.getActiveNetworkInfo();
                isVpnConnected = activeNetwork.getType() == ConnectivityManager.TYPE_VPN;
            }
        }
        return isVpnConnected;
    }

    public void startVpn(Context context) {
        ContextCompat.startForegroundService(context, new Intent(context, WireguardService.class));
        TimerUtils.cancelScheduledVpnAction(context);
    }

    public void stopVpn(Context context) {
        context.stopService(new Intent(context, WireguardService.class));
    }
}