// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.signin;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.accessibility.AccessibilityEvent;
import android.widget.FrameLayout;

import androidx.annotation.MainThread;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;

import org.chromium.base.BuildInfo;
import org.chromium.base.Promise;
import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.base.task.PostTask;
import org.chromium.base.task.TaskTraits;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.enterprise.util.EnterpriseInfo;
import org.chromium.chrome.browser.firstrun.FirstRunFragment;
import org.chromium.chrome.browser.firstrun.FirstRunUtils;
import org.chromium.chrome.browser.firstrun.MobileFreProgress;
import org.chromium.chrome.browser.firstrun.SkipTosDialogPolicyListener;
import org.chromium.chrome.browser.privacy.settings.PrivacyPreferencesManagerImpl;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileProvider;
import org.chromium.chrome.browser.ui.device_lock.DeviceLockCoordinator;
import org.chromium.chrome.browser.ui.signin.SigninUtils;
import org.chromium.chrome.browser.ui.signin.fullscreen_signin.FullscreenSigninConfig;
import org.chromium.chrome.browser.ui.signin.fullscreen_signin.FullscreenSigninCoordinator;
import org.chromium.chrome.browser.ui.signin.fullscreen_signin.FullscreenSigninMediator;
import org.chromium.chrome.browser.ui.signin.fullscreen_signin.FullscreenSigninView;
import org.chromium.components.browser_ui.device_lock.DeviceLockActivityLauncher;
import org.chromium.components.signin.AccountManagerFacadeProvider;
import org.chromium.components.signin.metrics.AccountConsistencyPromoAction;
import org.chromium.components.signin.metrics.SigninAccessPoint;
import org.chromium.ui.modaldialog.ModalDialogManager;
import org.chromium.ui.modaldialog.ModalDialogManagerHolder;

/** This fragment handles the sign-in without sync consent during the FRE. */
public class SigninFirstRunFragment extends Fragment
        implements FirstRunFragment,
                FullscreenSigninCoordinator.Delegate,
                DeviceLockCoordinator.Delegate {
    @VisibleForTesting static final int ADD_ACCOUNT_REQUEST_CODE = 1;

    // Used as a view holder for the current orientation of the device.
    private FrameLayout mFragmentView;
    private View mMainView;
    private ModalDialogManager mModalDialogManager;
    private SkipTosDialogPolicyListener mSkipTosDialogPolicyListener;
    private FullscreenSigninCoordinator mFullscreenSigninCoordinator;
    private DeviceLockCoordinator mDeviceLockCoordinator;
    private boolean mExitFirstRunCalled;
    private boolean mDelayedExitFirstRunCalledForTesting;
    private static final String GMS_PREFS = "chrome_gms_prefs";
    private static final String TAG = "SigninFirstRun";
    private boolean mGooglePlayServicesChecked = false;

    public SigninFirstRunFragment() {}

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        
        // Move Google Play Services check here, before any UI initialization
        if (!checkAndHandleGooglePlayServices()) {
            Log.w(TAG, "Google Play Services not available, stopping initialization");
            return;
        }
        try {
            mModalDialogManager = ((ModalDialogManagerHolder) getActivity()).getModalDialogManager();
            mFullscreenSigninCoordinator =
                    new FullscreenSigninCoordinator(
                            requireContext(),
                            mModalDialogManager,
                            this,
                            PrivacyPreferencesManagerImpl.getInstance(),
                            new FullscreenSigninConfig(
                                    /* shouldDisableSignin= */ BuildInfo.getInstance().isAutomotive),
                            SigninAccessPoint.START_PAGE);

            if (getPageDelegate().isLaunchedFromCct()) {
                mSkipTosDialogPolicyListener =
                        new SkipTosDialogPolicyListener(
                                getPageDelegate().getPolicyLoadListener(),
                                EnterpriseInfo.getInstance(),
                                null);
                mSkipTosDialogPolicyListener.onAvailable(
                        (Boolean skipTos) -> {
                            if (skipTos) exitFirstRun();
                        });
            }
        } catch (Exception e) {
            Log.e(TAG, "Error in onAttach: " + e.getMessage());
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mFragmentView = null;
        if (mSkipTosDialogPolicyListener != null) {
            mSkipTosDialogPolicyListener.destroy();
            mSkipTosDialogPolicyListener = null;
        }
        mFullscreenSigninCoordinator.destroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Keep device lock page if it's currently displayed.
        if (mDeviceLockCoordinator != null) {
            return;
        }
        // Inflate the view required for the current configuration and set it as the fragment view.
        mFragmentView.removeAllViews();
        mMainView =
                inflateFragmentView(
                        (LayoutInflater)
                                getActivity().getSystemService(Context.LAYOUT_INFLATER_SERVICE),
                        getActivity());
        mFragmentView.addView(mMainView);
    }

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        try {
            // Don't proceed if Google Play Services check failed
            if (!mGooglePlayServicesChecked) {
                Log.w(TAG, "Skipping view creation due to Google Play Services check failure");
                return new FrameLayout(getActivity());
            }

            mFragmentView = new FrameLayout(getActivity());
            mMainView = inflateFragmentView(inflater, getResources().getConfiguration());
            mFragmentView.addView(mMainView);
            return mFragmentView;
        } catch (Exception e) {
            Log.e(TAG, "Failed to create view: " + e.getMessage());
            return new FrameLayout(getActivity());
        }
    }

    private boolean checkAndHandleGooglePlayServices() {
        if (mGooglePlayServicesChecked) return true;
        
        try {
            Activity activity = getActivity();
            if (activity == null) {
                Log.e(TAG, "Activity is null during Google Play Services check");
                return false;
            }

            // Use the newer GoogleApiAvailability instead of deprecated GooglePlayServicesUtil
            com.google.android.gms.common.GoogleApiAvailability availability = 
                    com.google.android.gms.common.GoogleApiAvailability.getInstance();
            
            int result = availability.isGooglePlayServicesAvailable(activity);
            
            if (result != com.google.android.gms.common.ConnectionResult.SUCCESS) {
                Log.w(TAG, "Google Play Services not available, result: " + result);
                android.widget.Toast.makeText(
                        getActivity(),
                        "This app requires Google Play Services which are not available on this device. Some features may not work.",
                        android.widget.Toast.LENGTH_LONG).show();
                // Post to main thread to avoid window token issues
                activity.runOnUiThread(() -> {
                    try {
                        if (availability.isUserResolvableError(result)) {
                            // Show the default Google Play Services resolution dialog
                            availability.getErrorDialog(activity, result, 1000,
                                    dialog -> {
                                        // Dialog was cancelled
                                        showNonResolvableError();
                                    })
                                    .show();
                        } else {
                            showNonResolvableError();
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "Error showing Google Play Services dialog: " + e.getMessage());
                        showNonResolvableError();
                    }
                });
                return false;
            }
            
            mGooglePlayServicesChecked = true;
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Error checking Google Play Services: " + e.getMessage());
            showNonResolvableError();
            return false;
        }
    }

    private void showNonResolvableError() {
        Activity activity = getActivity();
        if (activity == null) return;

        try {
            activity.runOnUiThread(() -> {
                try {
                    if (activity.isFinishing()) return;
                    
                    androidx.appcompat.app.AlertDialog.Builder builder = 
                            new androidx.appcompat.app.AlertDialog.Builder(activity);
                    
                    builder.setTitle("Google Play Services Required")
                           .setMessage("Sorry, this app depends on Google Play Services which is not available on your device. The app cannot function without Google Play Services.")
                           .setPositiveButton("Exit", (dialog, which) -> {
                               dialog.dismiss();
                               activity.finishAffinity();
                           })
                           .setCancelable(false);

                    // Create dialog first to check for window token issues
                    androidx.appcompat.app.AlertDialog dialog = builder.create();
                    if (activity.getWindow() != null && !activity.isFinishing()) {
                        dialog.show();
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Fatal: Could not show error dialog: " + e.getMessage());
                    if (!activity.isFinishing()) {
                        activity.finishAffinity();
                    }
                }
            });
        } catch (Exception e) {
            Log.e(TAG, "Fatal: Error in showNonResolvableError: " + e.getMessage());
            if (activity != null && !activity.isFinishing()) {
                activity.finishAffinity();
            }
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        try {
            if (requestCode == ADD_ACCOUNT_REQUEST_CODE
                    && resultCode == Activity.RESULT_OK
                    && data != null) {
                String addedAccountName = data.getStringExtra(AccountManager.KEY_ACCOUNT_NAME);
                if (addedAccountName != null) {
                    mFullscreenSigninCoordinator.onAccountAdded(addedAccountName);
                }
            }
        } catch (Exception e) {
            android.util.Log.w("SigninFirstRun", "Failed to handle activity result: " + e.getMessage());
        }
    }

    /** Implements {@link FirstRunFragment}. */
    @Override
    public void setInitialA11yFocus() {
        // Ignore calls before view is created.
        if (getView() == null) return;

        final View title = getView().findViewById(R.id.title);
        title.sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_FOCUSED);
    }

    /** Implements {@link FirstRunFragment}. */
    @Override
    public void reset() {
        try {
            mFullscreenSigninCoordinator.reset();
        } catch (Exception e) {
            android.util.Log.w("SigninFirstRun", "Failed to reset: " + e.getMessage());
        }
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void addAccount() {
        try {
            getPageDelegate().recordFreProgressHistogram(MobileFreProgress.WELCOME_ADD_ACCOUNT);
            AccountManagerFacadeProvider.getInstance()
            .createAddAccountIntent(
                (@Nullable Intent intent) -> {
                    try {
                        if (intent != null) {
                            startActivityForResult(intent, ADD_ACCOUNT_REQUEST_CODE);
                            return;
                        }

                        // AccountManagerFacade couldn't create intent, use SigninUtils to open
                        // settings instead.
                        SigninUtils.openSettingsForAllAccounts(getActivity());
                    } catch (Exception e) {
                        android.util.Log.w("SigninFirstRun", "Failed to handle account: " + e.getMessage());
                    }
                });
        } catch (Exception e) {
            android.util.Log.w("SigninFirstRun", "Failed to add account: " + e.getMessage());
        }
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void acceptTermsOfService(boolean allowMetricsAndCrashUploading) {
        try {
            getPageDelegate().acceptTermsOfService(allowMetricsAndCrashUploading);
        } catch (Exception e) {
            android.util.Log.w("SigninFirstRun", "Failed to accept terms: " + e.getMessage());
        }
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void advanceToNextPage() {
        getPageDelegate().advanceToNextPage();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void recordUserSignInHistograms(@AccountConsistencyPromoAction int promoAction) {
        @MobileFreProgress
        int progressState =
                promoAction == AccountConsistencyPromoAction.SIGNED_IN_WITH_DEFAULT_ACCOUNT
                        ? MobileFreProgress.WELCOME_SIGNIN_WITH_DEFAULT_ACCOUNT
                        : MobileFreProgress.WELCOME_SIGNIN_WITH_NON_DEFAULT_ACCOUNT;
        getPageDelegate().recordFreProgressHistogram(progressState);
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void recordSigninDismissedHistograms() {
        getPageDelegate().recordFreProgressHistogram(MobileFreProgress.WELCOME_DISMISS);
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void recordLoadCompletedHistograms(
            @FullscreenSigninMediator.LoadPoint int slowestLoadPoint) {
        getPageDelegate().recordLoadCompletedHistograms(slowestLoadPoint);
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void recordNativeInitializedHistogram() {
        getPageDelegate().recordNativeInitializedHistogram();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void showInfoPage(@StringRes int url) {
        getPageDelegate().showInfoPage(url);
    }

    @Override
    public OneshotSupplier<ProfileProvider> getProfileSupplier() {
        return getPageDelegate().getProfileProviderSupplier();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public OneshotSupplier<Boolean> getPolicyLoadListener() {
        return getPageDelegate().getPolicyLoadListener();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public OneshotSupplier<Boolean> getChildAccountStatusSupplier() {
        return getPageDelegate().getChildAccountStatusSupplier();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public Promise<Void> getNativeInitializationPromise() {
        return getPageDelegate().getNativeInitializationPromise();
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public boolean shouldDisplayManagementNoticeOnManagedDevices() {
        return true;
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public boolean shouldDisplayFooterText() {
        return true;
    }

    @MainThread
    private void exitFirstRun() {
        // Make sure this function is called at most once.
        if (!mExitFirstRunCalled) {
            mExitFirstRunCalled = true;
            PostTask.postDelayedTask(
                    TaskTraits.UI_DEFAULT,
                    () -> {
                        mDelayedExitFirstRunCalledForTesting = true;

                        // If we've been detached, someone else has handled something, and it's no
                        // longer clear that we should still be accepting the ToS and exiting the
                        // FRE.
                        if (isDetached()) return;

                        getPageDelegate().acceptTermsOfService(false);
                        getPageDelegate().exitFirstRun();
                    },
                    FirstRunUtils.getSkipTosExitDelayMs());
        }
    }

    private View inflateFragmentView(LayoutInflater inflater, Activity activity) {
        try {
            boolean useLandscapeLayout = SigninUtils.shouldShowDualPanesHorizontalLayout(activity);

            final FullscreenSigninView view =
                    (FullscreenSigninView)
                            inflater.inflate(
                                    useLandscapeLayout
                                            ? R.layout.fullscreen_signin_landscape_view
                                            : R.layout.fullscreen_signin_portrait_view,
                                    null,
                                    false);
            mFullscreenSigninCoordinator.setView(view);
            return view;
        } catch (Exception e) {
            android.util.Log.w("SigninFirstRun", "Failed to inflate view: " + e.getMessage());
            // Return an empty view to prevent crash
            return new FrameLayout(getActivity());
        }
    }

    /** Implements {@link FullscreenSigninCoordinator.Delegate}. */
    @Override
    public void displayDeviceLockPage(Account selectedAccount) {
        Profile profile = ProfileProvider.getOrCreateProfile(getProfileSupplier().get(), false);
        mDeviceLockCoordinator =
                new DeviceLockCoordinator(
                        this,
                        getPageDelegate().getWindowAndroid(),
                        profile,
                        getActivity(),
                        selectedAccount);
    }

    /** Implements {@link DeviceLockCoordinator.Delegate}. */
    @Override
    public void setView(View view) {
        mFragmentView.removeAllViews();
        mFragmentView.addView(view);
    }

    /** Implements {@link DeviceLockCoordinator.Delegate}. */
    @Override
    public void onDeviceLockReady() {
        if (mFragmentView != null) {
            restoreMainView();
        }
        if (mDeviceLockCoordinator != null) {
            mDeviceLockCoordinator.destroy();
            mDeviceLockCoordinator = null;

            // Hold off on continuing sign-in if the delegate is null (due to the host activity
            // being killed in the background.
            if (getPageDelegate() != null) {
                mFullscreenSigninCoordinator.continueSignIn();
            }
        }
    }

    /** Implements {@link DeviceLockCoordinator.Delegate}. */
    @Override
    public void onDeviceLockRefused() {
        mFullscreenSigninCoordinator.cancelSignInAndDismiss();
    }

    @Override
    public @DeviceLockActivityLauncher.Source String getSource() {
        return DeviceLockActivityLauncher.Source.FIRST_RUN;
    }

    private void restoreMainView() {
        mFragmentView.removeAllViews();
        mFragmentView.addView(mMainView);
    }

    boolean getDelayedExitFirstRunCalledForTesting() {
        return mDelayedExitFirstRunCalledForTesting;
    }
}
