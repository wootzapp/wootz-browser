// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.signin.fullscreen_signin;
import android.content.SharedPreferences;
import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.cardview.widget.CardView;
import android.util.Log;

import org.chromium.chrome.browser.ui.signin.R;
import org.chromium.ui.widget.ButtonCompat;
import org.chromium.ui.widget.TextViewWithClickableSpans;

/** View that wraps the fullscreen signin promo and caches references to UI elements. */
public class FullscreenSigninView extends RelativeLayout {
    private ImageView mLogo;
    private TextView mTitle;
    private TextView mSubtitle;
    private View mBrowserManagedHeader;
    private TextView mPrivacyDisclaimer;
    private ProgressBar mInitialLoadProgressSpinner;
    private ViewGroup mSelectedAccount;
    private ImageView mExpandIcon;
    private ButtonCompat mContinueButton;
    private ButtonCompat mDismissButton;
    private TextViewWithClickableSpans mFooter;
    private ProgressBar mSigninProgressSpinner;
    private TextView mSigninProgressText;
    private View mRebrandingCard;
    private TextView mRebrandingText;

    public FullscreenSigninView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mLogo = findViewById(R.id.fre_logo);
        mTitle = findViewById(R.id.title);
        mSubtitle = findViewById(R.id.subtitle);
        mBrowserManagedHeader = findViewById(R.id.fre_browser_managed_by);
        mInitialLoadProgressSpinner =
                findViewById(R.id.fre_native_and_policy_load_progress_spinner);
        mSelectedAccount = findViewById(R.id.signin_fre_selected_account);
        mExpandIcon = findViewById(R.id.signin_fre_selected_account_expand_icon);
        mContinueButton = findViewById(R.id.signin_fre_continue_button);
        mDismissButton = findViewById(R.id.signin_fre_dismiss_button);
        mFooter = findViewById(R.id.signin_fre_footer);
        mSigninProgressSpinner = findViewById(R.id.fre_signin_progress_spinner);
        mSigninProgressText = findViewById(R.id.fre_signin_progress_text);
        mPrivacyDisclaimer = (TextView) findViewById(R.id.privacy_disclaimer);
        mRebrandingCard = findViewById(R.id.rebranding_card);
        mRebrandingText = findViewById(R.id.rebranding_text);
        
        // Delay checking for rebranding data to ensure SharedPreferences is ready
        postDelayed(this::setupRebrandingInfo, 800); // Wait 800ms
    }

    private void setupRebrandingInfo() {
        
        if (mRebrandingCard == null || mRebrandingText == null) return;
        
        // Default visibility is GONE
        mRebrandingCard.setVisibility(View.GONE);
        
        // Access SharedPreferences
        SharedPreferences prefs = getContext().getSharedPreferences("branch_data", Context.MODE_PRIVATE);
        String channel = prefs.getString("utm_source_wootzapp", "");
        Log.e("FullscreenSigninView", "Read channel from prefs: '" + channel + "'");
        
        // Only show card if channel value exists
        if (channel != null && !channel.isEmpty()) {
            mRebrandingCard.setVisibility(View.VISIBLE);
            Log.e("FullscreenSigninView", "Channel exists, set card to VISIBLE");
            
            // Customize message based on channel value
            String message = "After clicking Get Started, WootzApp will rebrand as " + 
                             channel + " and close. You can find it in your app drawer!";
            mRebrandingText.setText(message);
        } else {
            Log.e("FullscreenSigninView", "Channel is empty or null, card stays GONE");
        }
    }

    View getBrowserManagedHeaderView() {
        return mBrowserManagedHeader;
    }

    TextView getPrivacyDisclaimer() {
        return mPrivacyDisclaimer;
    }

    ProgressBar getInitialLoadProgressSpinnerView() {
        return mInitialLoadProgressSpinner;
    }

    ViewGroup getSelectedAccountView() {
        return mSelectedAccount;
    }

    ImageView getExpandIconView() {
        return mExpandIcon;
    }

    ButtonCompat getContinueButtonView() {
        return mContinueButton;
    }

    ButtonCompat getDismissButtonView() {
        return mDismissButton;
    }

    TextViewWithClickableSpans getFooterView() {
        return mFooter;
    }

    ProgressBar getSigninProgressSpinner() {
        return mSigninProgressSpinner;
    }

    TextView getSigninProgressText() {
        return mSigninProgressText;
    }

    TextView getSubtitle() {
        return mSubtitle;
    }

    TextView getTitle() {
        return mTitle;
    }

    ImageView getLogo() {
        return mLogo;
    }
}
