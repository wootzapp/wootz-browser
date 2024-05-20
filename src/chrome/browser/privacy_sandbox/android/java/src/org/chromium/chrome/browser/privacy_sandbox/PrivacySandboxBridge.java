// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.privacy_sandbox;

import org.jni_zero.CalledByNative;
import org.jni_zero.JniType;
import org.jni_zero.NativeMethods;

import org.chromium.base.Callback;
import org.chromium.chrome.browser.profiles.Profile;

import java.util.Arrays;
import java.util.List;

/** Bridge, providing access to the native-side Privacy Sandbox configuration. */
public class PrivacySandboxBridge {

    private final Profile mProfile;

    public PrivacySandboxBridge(Profile profile) {
        mProfile = profile;
    }

    public boolean isPrivacySandboxRestricted() {
        return PrivacySandboxBridgeJni.get().isPrivacySandboxRestricted(mProfile);
    }

    public boolean isRestrictedNoticeEnabled() {
        return PrivacySandboxBridgeJni.get().isRestrictedNoticeEnabled(mProfile);
    }

    public List<Topic> getCurrentTopTopics() {
        return sortTopics(PrivacySandboxBridgeJni.get().getCurrentTopTopics(mProfile));
    }

    public List<Topic> getBlockedTopics() {
        return sortTopics(PrivacySandboxBridgeJni.get().getBlockedTopics(mProfile));
    }

    public List<Topic> getFirstLevelTopics() {
        return sortTopics(PrivacySandboxBridgeJni.get().getFirstLevelTopics(mProfile));
    }

    public List<Topic> getChildTopicsCurrentlyAssigned(Topic topic) {
        return sortTopics(
                PrivacySandboxBridgeJni.get()
                        .getChildTopicsCurrentlyAssigned(
                                mProfile, topic.getTopicId(), topic.getTaxonomyVersion()));
    }

    public void setTopicAllowed(Topic topic, boolean allowed) {
        PrivacySandboxBridgeJni.get()
                .setTopicAllowed(mProfile, topic.getTopicId(), topic.getTaxonomyVersion(), allowed);
    }

    @CalledByNative
    private static Topic createTopic(
            int topicId, int taxonomyVersion, String name, String description) {
        return new Topic(topicId, taxonomyVersion, name, description);
    }

    private static List<Topic> sortTopics(Object[] topics) {
        Arrays.sort(
                topics,
                (o1, o2) -> {
                    return ((Topic) o1).getName().compareTo(((Topic) o2).getName());
                });
        return (List<Topic>) (List<?>) Arrays.asList(topics);
    }

    public void getFledgeJoiningEtldPlusOneForDisplay(Callback<List<String>> callback) {
        Callback<String[]> arrayCallback =
                (String[] domains) -> callback.onResult(Arrays.asList(domains));
        PrivacySandboxBridgeJni.get()
                .getFledgeJoiningEtldPlusOneForDisplay(mProfile, arrayCallback);
    }

    public List<String> getBlockedFledgeJoiningTopFramesForDisplay() {
        return Arrays.asList(
                PrivacySandboxBridgeJni.get().getBlockedFledgeJoiningTopFramesForDisplay(mProfile));
    }

    public void setFledgeJoiningAllowed(String topFrameEtldPlus1, boolean allowed) {
        PrivacySandboxBridgeJni.get().setFledgeJoiningAllowed(mProfile, topFrameEtldPlus1, allowed);
    }

    public @PromptType int getRequiredPromptType() {
        return PrivacySandboxBridgeJni.get().getRequiredPromptType(mProfile);
    }

    public void promptActionOccurred(@PromptAction int action) {
        PrivacySandboxBridgeJni.get().promptActionOccurred(mProfile, action);
    }

    public boolean isFirstPartySetsDataAccessEnabled() {
        return PrivacySandboxBridgeJni.get().isFirstPartySetsDataAccessEnabled(mProfile);
    }

    public boolean isFirstPartySetsDataAccessManaged() {
        return PrivacySandboxBridgeJni.get().isFirstPartySetsDataAccessManaged(mProfile);
    }

    public boolean isPartOfManagedFirstPartySet(String origin) {
        return PrivacySandboxBridgeJni.get().isPartOfManagedFirstPartySet(mProfile, origin);
    }

    public void setFirstPartySetsDataAccessEnabled(boolean enabled) {
        PrivacySandboxBridgeJni.get().setFirstPartySetsDataAccessEnabled(mProfile, enabled);
    }

    /**
     * Gets the First Party Sets owner hostname given a FPS member origin.
     *
     * @param memberOrigin FPS member origin.
     * @return A string containing the owner hostname, null if it doesn't exist.
     */
    public String getFirstPartySetOwner(String memberOrigin) {
        return PrivacySandboxBridgeJni.get().getFirstPartySetOwner(mProfile, memberOrigin);
    }

    public void topicsToggleChanged(boolean newValue) {
        PrivacySandboxBridgeJni.get().topicsToggleChanged(mProfile, newValue);
    }

    public void setAllPrivacySandboxAllowedForTesting() {
        PrivacySandboxBridgeJni.get().setAllPrivacySandboxAllowedForTesting(mProfile); // IN-TEST
    }

    @NativeMethods
    public interface Natives {
        boolean isPrivacySandboxRestricted(Profile profile);

        boolean isRestrictedNoticeEnabled(Profile profile);

        boolean isFirstPartySetsDataAccessEnabled(Profile profile);

        boolean isFirstPartySetsDataAccessManaged(Profile profile);

        boolean isPartOfManagedFirstPartySet(Profile profile, String origin);

        void setFirstPartySetsDataAccessEnabled(Profile profile, boolean enabled);

        String getFirstPartySetOwner(Profile profile, String memberOrigin);

        @JniType("std::vector")
        Object[] getCurrentTopTopics(Profile profile);

        @JniType("std::vector")
        Object[] getBlockedTopics(Profile profile);

        @JniType("std::vector")
        Object[] getFirstLevelTopics(Profile profile);

        @JniType("std::vector")
        Object[] getChildTopicsCurrentlyAssigned(Profile profile, int topicId, int taxonomyVersion);

        void setTopicAllowed(Profile profile, int topicId, int taxonomyVersion, boolean allowed);

        void getFledgeJoiningEtldPlusOneForDisplay(Profile profile, Callback<String[]> callback);

        String[] getBlockedFledgeJoiningTopFramesForDisplay(Profile profile);

        void setFledgeJoiningAllowed(Profile profile, String topFrameEtldPlus1, boolean allowed);

        int getRequiredPromptType(Profile profile);

        void promptActionOccurred(Profile profile, int action);

        void topicsToggleChanged(Profile profile, boolean newValue);

        void setAllPrivacySandboxAllowedForTesting(Profile profile); // IN-TEST
    }
}
