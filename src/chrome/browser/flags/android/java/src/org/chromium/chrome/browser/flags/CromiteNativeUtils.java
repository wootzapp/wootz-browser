package org.chromium.chrome.browser.flags;

import org.chromium.base.cached_flags.CachedFlag;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.base.cached_flags.CachedFlagsSharedPreferences;

import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;

public class CromiteNativeUtils {
    /**
     * Allows the modification of the flag value on the java side.
     * Currently only the feature flag with on / off values is managed.
     *
     * @param featureName the feature name from ChromeFeatureList.
     * @param flagName the flag name name from about_flags.cc.
     */
    public static void setFlagEnabled(String flagName, Boolean newValue) {
        CachedFlag cachedFlag = ChromeFeatureList.sAllCachedFlags.get(flagName);
        setFlagEnabled(cachedFlag == null
                        ? ""
                        : cachedFlag.getFeatureName(), flagName, newValue);
    }

    public static void setFlagEnabled(String featureName, String flagName, Boolean newValue) {
        CromiteNativeUtilsJni.get().setEnabled(flagName, newValue);

        if (!featureName.isEmpty()) {
            CachedFlag cachedFlag = ChromeFeatureList.sAllCachedFlags.get(featureName);
            String preferenceName = cachedFlag.getSharedPreferenceKey();

            CachedFlagsSharedPreferences.getInstance().writeBoolean(preferenceName, newValue);
            cachedFlag.setForTesting(newValue);
        }
    }

    public static boolean isFlagEnabled(String featureName) {
        return CromiteNativeUtilsJni.get().isFlagEnabled(featureName);
    }

    @NativeMethods
    interface Natives {
        void setEnabled(String featureName, boolean newValue);
        boolean isFlagEnabled(String featureName);
    }
}