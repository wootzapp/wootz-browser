package org.chromium.chrome.browser.vpn.timer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import org.chromium.chrome.browser.vpn.utils.WootzVpnProfileUtils;

public class TimerVpnActionReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            WootzVpnProfileUtils.getInstance().startVpn(context);
        } catch (Exception exc) {
            // There could be uninitialized parts on early stages. Just ignore it the exception,
            // it's better comparing to crashing
        }
    }
}