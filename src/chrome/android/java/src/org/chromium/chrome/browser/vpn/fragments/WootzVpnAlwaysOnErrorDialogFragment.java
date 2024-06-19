package org.chromium.chrome.browser.vpn.fragments;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.WootzDialogFragment;

public class WootzVpnAlwaysOnErrorDialogFragment
        extends WootzDialogFragment implements View.OnClickListener {
    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(
                R.layout.fragment_wootz_vpn_always_on_error_dialog, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        Button openVpnSettings = view.findViewById(R.id.btn_done);
        openVpnSettings.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.btn_done) {
            Intent intent = new Intent("android.net.vpn.SETTINGS");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
        dismiss();
    }
}