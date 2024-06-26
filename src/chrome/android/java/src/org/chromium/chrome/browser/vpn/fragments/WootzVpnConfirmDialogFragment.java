package org.chromium.chrome.browser.vpn.fragments;

import android.animation.Animator;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.WootzDialogFragment;

public class WootzVpnConfirmDialogFragment extends WootzDialogFragment {

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_wootz_vpn_confirm_dialog, container, false);
    }
}