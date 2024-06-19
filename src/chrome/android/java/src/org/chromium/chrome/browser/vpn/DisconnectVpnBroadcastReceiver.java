package org.chromium.chrome.browser.vpn;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

// import org.chromium.chrome.browser.vpn.wireguard.WireguardService;

public class DisconnectVpnBroadcastReceiver extends BroadcastReceiver {
    public static String DISCONNECT_VPN_ACTION = "disconnect_vpn_action";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        // if (action != null && action.equals(DISCONNECT_VPN_ACTION)) {
        //     context.stopService(new Intent(context, WireguardService.class));
        // }
    }
}
