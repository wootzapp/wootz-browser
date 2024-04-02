// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.cached_flags;

import androidx.annotation.AnyThread;

import org.json.JSONException;
import org.json.JSONObject;

import org.chromium.base.FeatureMap;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/** AllCachedFieldTrialParameters caches all the parameters for a feature. */
public class AllCachedFieldTrialParameters extends CachedFieldTrialParameter {
    /** Decodes a previously encoded map. Returns empty map on parse error. */
    private static Map<String, String> decodeJsonEncodedMap(String value) {
        Map<String, String> resultingMap = new HashMap<String, String>();
        try {
            final JSONObject json = new JSONObject(value);
            Iterator<String> keys = json.keys();
            while (keys.hasNext()) {
                final String key = keys.next();
                resultingMap.put(key, json.getString(key));
            }
            return resultingMap;
        } catch (JSONException e) {
        }
        return new HashMap<>();
    }

    private static String encodeParams(Map<String, String> params) {
        return new JSONObject(params).toString();
    }

    public AllCachedFieldTrialParameters(FeatureMap featureMap, String featureName) {
        // As this includes all parameters, the parameterName is empty.
        super(featureMap, featureName, /* parameterName= */ "", FieldTrialParameterType.ALL);
    }

    /** Returns a map of field trial parameter to value. */
    @AnyThread
    public Map<String, String> getParams() {
        return decodeJsonEncodedMap(getConsistentStringValue());
    }

    @AnyThread
    private String getConsistentStringValue() {
        CachedFlagsSafeMode.getInstance().onFlagChecked();

        String preferenceName = getSharedPreferenceKey();

        String value = ValuesOverridden.getString(preferenceName);
        if (value != null) {
            return value;
        }

        synchronized (ValuesReturned.sStringValues) {
            value = ValuesReturned.sStringValues.get(preferenceName);
            if (value != null) {
                return value;
            }

            value = CachedFlagsSafeMode.getInstance().getStringFieldTrialParam(preferenceName, "");
            if (value == null) {
                value = CachedFlagsSharedPreferences.getInstance().readString(preferenceName, "");
            }

            ValuesReturned.sStringValues.put(preferenceName, value);
        }
        return value;
    }

    @Override
    void cacheToDisk() {
        final Map<String, String> params =
                mFeatureMap.getFieldTrialParamsForFeature(getFeatureName());
        CachedFlagsSharedPreferences.getInstance()
                .writeString(getSharedPreferenceKey(), encodeParams(params));
    }

    /** Sets the parameters for the specified feature when used in tests. */
    public static void setForTesting(String featureName, Map<String, String> params) {
        String preferenceKey = generateSharedPreferenceKey(featureName, "");
        String overrideValue = encodeParams(params);
        ValuesOverridden.setOverrideForTesting(preferenceKey, overrideValue);
    }
}
