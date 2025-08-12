// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid

package org.chromium.chrome.browser.wootzapp_search;

import org.chromium.base.shared_preferences.SharedPreferencesManager;
import android.util.Log;
import org.chromium.base.ContextUtils;

public class AiConfig {
    private static final String PREF_AI_MODEL = "wootzapp_search_ai_model_name";
    private static final String PREF_AI_API_KEY = "wootzapp_search_ai_model_api_key";
    private static final String EXTENSION_ID = "lhgoolpdddhhfahbnofaomjhfhfjfhop";

    public static String getApiKey() {

        String apiKey = ContextUtils.getAppSharedPreferences().getString(PREF_AI_API_KEY, "");
        return apiKey;
    }

    public static String getAiModel() {
        String aiModel = ContextUtils.getAppSharedPreferences().getString(PREF_AI_MODEL, "");
        return aiModel;
    }

    /**
     * Clears AI preferences After Uninstalling the WootzApp Search Extension
     * @param extensionId The ID of the extension being uninstalled
     */
    public static void clearAiPreferencesForExtension(String extensionId) {
        Log.d("AiConfig", "Clearing AI preferences for extension: " + extensionId);
        if (extensionId.equals(EXTENSION_ID)) {
            ContextUtils.getAppSharedPreferences().edit()
                .remove(PREF_AI_MODEL)
                .remove(PREF_AI_API_KEY)
                .apply();
        }
    }

}