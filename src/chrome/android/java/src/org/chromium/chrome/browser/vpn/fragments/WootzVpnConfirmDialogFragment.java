package org.chromium.chrome.browser.vpn.fragments;

import android.animation.Animator;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.airbnb.lottie.LottieAnimationView;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.WootzDialogFragment;

public class WootzVpnConfirmDialogFragment extends WootzDialogFragment {
    private LottieAnimationView mAnimatedView;

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_wootz_vpn_confirm_dialog, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        mAnimatedView = view.findViewById(R.id.bg_image);
        mAnimatedView.setAnimation("wootz_vpn_confirm.json");
        mAnimatedView.addAnimatorListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animation) {}

            @Override
            public void onAnimationEnd(Animator animation) {
                try {
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                getActivity().finish();
            }

            @Override
            public void onAnimationCancel(Animator animation) {}

            @Override
            public void onAnimationRepeat(Animator animation) {}
        });
        mAnimatedView.playAnimation();
    }
}