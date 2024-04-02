// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.signin.history_sync;

import android.content.Context;
import android.text.TextUtils;

import org.chromium.base.metrics.RecordHistogram;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.signin.services.IdentityServicesProvider;
import org.chromium.chrome.browser.signin.services.ProfileDataCache;
import org.chromium.chrome.browser.signin.services.SigninManager;
import org.chromium.chrome.browser.sync.SyncServiceFactory;
import org.chromium.components.signin.base.CoreAccountInfo;
import org.chromium.components.signin.identitymanager.ConsentLevel;
import org.chromium.components.signin.metrics.SigninAccessPoint;
import org.chromium.components.sync.SyncService;
import org.chromium.components.sync.UserSelectableType;
import org.chromium.ui.modelutil.PropertyModel;

class HistorySyncMediator implements ProfileDataCache.Observer, SigninManager.SignInStateObserver {
    private final PropertyModel mModel;
    private final String mAccountEmail;
    private final HistorySyncCoordinator.HistorySyncDelegate mDelegate;
    private final SigninManager mSigninManager;
    private final SyncService mSyncService;
    private ProfileDataCache mProfileDataCache;
    private final @SigninAccessPoint int mAccessPoint;

    HistorySyncMediator(
            Context context,
            HistorySyncCoordinator.HistorySyncDelegate delegate,
            Profile profile,
            @SigninAccessPoint int accessPoint) {
        mAccessPoint = accessPoint;
        mDelegate = delegate;
        mProfileDataCache = ProfileDataCache.createWithDefaultImageSizeAndNoBadge(context);
        mSigninManager = IdentityServicesProvider.get().getSigninManager(profile);
        mSyncService = SyncServiceFactory.getForProfile(profile);
        mProfileDataCache.addObserver(this);
        mSigninManager.addSignInStateObserver(this);
        mAccountEmail =
                CoreAccountInfo.getEmailFrom(
                        mSigninManager
                                .getIdentityManager()
                                .getPrimaryAccountInfo(ConsentLevel.SIGNIN));
        // The history sync screen should never be created when the user is signed out.
        assert mAccountEmail != null;
        mModel =
                HistorySyncProperties.createModel(
                        mProfileDataCache.getProfileDataOrDefault(mAccountEmail),
                        this::onAcceptClicked,
                        this::onDeclineClicked);
    }

    /** Implements {@link ProfileDataCache.Observer}. */
    @Override
    public void onProfileDataUpdated(String accountEmail) {
        if (!TextUtils.equals(mAccountEmail, accountEmail)) {
            return;
        }
        mModel.set(
                HistorySyncProperties.PROFILE_DATA,
                mProfileDataCache.getProfileDataOrDefault(accountEmail));
    }

    /** Implements {@link SigninManager.SignInStateObserver} */
    @Override
    public void onSignedOut() {
        RecordHistogram.recordEnumeratedHistogram(
                "Signin.HistorySyncOptIn.Aborted", mAccessPoint, SigninAccessPoint.MAX);
        mDelegate.dismissHistorySync();
    }

    void destroy() {
        mProfileDataCache.removeObserver(this);
        mSigninManager.removeSignInStateObserver(this);
    }

    PropertyModel getModel() {
        return mModel;
    }

    private void onAcceptClicked() {
        RecordHistogram.recordEnumeratedHistogram(
                "Signin.HistorySyncOptIn.Completed", mAccessPoint, SigninAccessPoint.MAX);
        mSyncService.setSelectedType(UserSelectableType.HISTORY, /* isTypeOn= */ true);
        mSyncService.setSelectedType(UserSelectableType.TABS, /* isTypeOn= */ true);
        mDelegate.dismissHistorySync();
    }

    private void onDeclineClicked() {
        RecordHistogram.recordEnumeratedHistogram(
                "Signin.HistorySyncOptIn.Declined", mAccessPoint, SigninAccessPoint.MAX);
        mDelegate.dismissHistorySync();
    }
}
