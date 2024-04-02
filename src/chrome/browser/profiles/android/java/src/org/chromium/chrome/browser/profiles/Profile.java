// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.profiles;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;

import org.chromium.chrome.browser.cookies.CookiesFetcher;
import org.chromium.components.profile_metrics.BrowserProfileType;
import org.chromium.content_public.browser.BrowserContextHandle;
import org.chromium.content_public.browser.WebContents;

/** Wrapper that allows passing a Profile reference around in the Java layer. */
public class Profile implements BrowserContextHandle {
    /** Holds OTRProfileID for OffTheRecord profiles. Is null for regular profiles. */
    @Nullable private final OTRProfileID mOTRProfileID;

    /** Pointer to the Native-side ProfileAndroid. */
    private long mNativeProfileAndroid;

    private Profile(long nativeProfileAndroid) {
        mNativeProfileAndroid = nativeProfileAndroid;
        if (ProfileJni.get().isOffTheRecord(mNativeProfileAndroid, Profile.this)) {
            mOTRProfileID = ProfileJni.get().getOTRProfileID(mNativeProfileAndroid, Profile.this);
        } else {
            mOTRProfileID = null;
        }
    }

    /**
     * @param webContents {@link WebContents} object.
     * @return {@link Profile} object associated with the given WebContents.
     */
    public static Profile fromWebContents(WebContents webContents) {
        return (Profile) ProfileJni.get().fromWebContents(webContents);
    }

    /**
     * Handles type conversion of Java side {@link BrowserContextHandle} to {@link Profile}.
     * @param browserContextHandle Java reference to native BrowserContext.
     * @return A strongly typed reference the {@link Profile}.
     */
    public static Profile fromBrowserContextHandle(BrowserContextHandle browserContextHandle) {
        return (Profile) browserContextHandle;
    }

    /**
     * Returns the {@link BrowserProfileType} for the corresponding profile.
     *
     * Please note {@link BrowserProfileType} is generated from native so, it also contains other
     * types of Profile like Guest and System that we don't support in Android.
     */
    public static @BrowserProfileType int getBrowserProfileTypeFromProfile(Profile profile) {
        assert profile != null;

        if (!profile.isOffTheRecord()) return BrowserProfileType.REGULAR;
        if (profile.isPrimaryOTRProfile()) return BrowserProfileType.INCOGNITO;
        return BrowserProfileType.OTHER_OFF_THE_RECORD_PROFILE;
    }

    public Profile getOriginalProfile() {
        return (Profile) ProfileJni.get().getOriginalProfile(mNativeProfileAndroid, Profile.this);
    }

    /**
     * Returns the OffTheRecord profile with given OTRProfileiD. If the profile
     * does not exist and createIfNeeded is true, a new profile is created,
     * otherwise returns null.
     *
     * @param profileID {@link OTRProfileID} object.
     * @param createIfNeeded Boolean indicating the profile should be created if doesn't exist.
     */
    public Profile getOffTheRecordProfile(OTRProfileID profileID, boolean createIfNeeded) {
        assert profileID != null;
        return (Profile)
                ProfileJni.get()
                        .getOffTheRecordProfile(
                                mNativeProfileAndroid, Profile.this, profileID, createIfNeeded);
    }

    /**
     * Returns the OffTheRecord profile for incognito tabs.  If the profile
     * does not exist and createIfNeeded is true, a new profile is created,
     * otherwise returns null.
     *
     * @param createIfNeeded Boolean indicating the profile should be created if doesn't exist.
     */
    public Profile getPrimaryOTRProfile(boolean createIfNeeded) {
        return (Profile)
                ProfileJni.get()
                        .getPrimaryOTRProfile(mNativeProfileAndroid, Profile.this, createIfNeeded);
    }

    /**
     * Returns the OffTheRecord profile id for OffTheRecord profiles, and null for regular profiles.
     */
    public @Nullable OTRProfileID getOTRProfileID() {
        return mOTRProfileID;
    }

    /**
     * Returns if OffTheRecord profile with given OTRProfileID exists.
     *
     * @param profileID {@link OTRProfileID} object.
     */
    public boolean hasOffTheRecordProfile(OTRProfileID profileID) {
        assert profileID != null;
        return ProfileJni.get()
                .hasOffTheRecordProfile(mNativeProfileAndroid, Profile.this, profileID);
    }

    /** Returns if primary OffTheRecord profile exists. */
    public boolean hasPrimaryOTRProfile() {
        return ProfileJni.get().hasPrimaryOTRProfile(mNativeProfileAndroid, Profile.this);
    }

    /** Returns if the profile is a primary OTR Profile. */
    public boolean isPrimaryOTRProfile() {
        return ProfileJni.get().isPrimaryOTRProfile(mNativeProfileAndroid, Profile.this);
    }

    public ProfileKey getProfileKey() {
        return (ProfileKey) ProfileJni.get().getProfileKey(mNativeProfileAndroid, Profile.this);
    }

    public boolean isOffTheRecord() {
        return mOTRProfileID != null;
    }

    /**
     * @return Whether the profile is signed in to a child account.
     * @deprecated Please use {@link
     *     org.chromium.components.signin.base.AccountCapabilities#isSubjectToParentalControls}
     *     instead.
     */
    @Deprecated
    public boolean isChild() {
        return ProfileJni.get().isChild(mNativeProfileAndroid, Profile.this);
    }

    /** Wipes all data for this profile. */
    public void wipe() {
        ProfileJni.get().wipe(mNativeProfileAndroid, Profile.this);
    }

    /**
     * @return Whether or not the native side profile exists.
     */
    @VisibleForTesting
    public boolean isNativeInitialized() {
        return mNativeProfileAndroid != 0;
    }

    /**
     * When called, raises an exception if the native pointer is not initialized. This is useful to
     * get a more debuggable stacktrace than failing on native-side when dereferencing.
     */
    public void ensureNativeInitialized() {
        if (mNativeProfileAndroid == 0) {
            throw new RuntimeException("Native profile pointer not initialized.");
        }
    }

    @Override
    public long getNativeBrowserContextPointer() {
        return ProfileJni.get().getBrowserContextPointer(mNativeProfileAndroid);
    }

    @CalledByNative
    private static Profile create(long nativeProfileAndroid) {
        return new Profile(nativeProfileAndroid);
    }

    @CalledByNative
    private void onNativeDestroyed() {
        mNativeProfileAndroid = 0;

        if (mOTRProfileID != null) {
            CookiesFetcher.deleteCookiesIfNecessary();
        }

        ProfileManager.onProfileDestroyed(this);
    }

    @CalledByNative
    private long getNativePointer() {
        return mNativeProfileAndroid;
    }

    @NativeMethods
    public interface Natives {
        Object fromWebContents(WebContents webContents);

        Object getOriginalProfile(long nativeProfileAndroid, Profile caller);

        Object getOffTheRecordProfile(
                long nativeProfileAndroid,
                Profile caller,
                OTRProfileID otrProfileID,
                boolean createIfNeeded);

        Object getPrimaryOTRProfile(
                long nativeProfileAndroid, Profile caller, boolean createIfNeeded);

        boolean hasOffTheRecordProfile(
                long nativeProfileAndroid, Profile caller, OTRProfileID otrProfileID);

        boolean hasPrimaryOTRProfile(long nativeProfileAndroid, Profile caller);

        boolean isOffTheRecord(long nativeProfileAndroid, Profile caller);

        boolean isPrimaryOTRProfile(long nativeProfileAndroid, Profile caller);

        boolean isChild(long nativeProfileAndroid, Profile caller);

        void wipe(long nativeProfileAndroid, Profile caller);

        Object getProfileKey(long nativeProfileAndroid, Profile caller);

        long getBrowserContextPointer(long nativeProfileAndroid);

        OTRProfileID getOTRProfileID(long nativeProfileAndroid, Profile caller);
    }
}
