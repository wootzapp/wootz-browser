// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions.ai;

import androidx.annotation.VisibleForTesting;

import org.chromium.chrome.browser.omnibox.styles.SuggestionSpannable;
import org.chromium.chrome.browser.omnibox.suggestions.base.BaseSuggestionViewProperties;
import org.chromium.ui.modelutil.PropertyKey;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.ui.modelutil.PropertyModel.WritableBooleanPropertyKey;
import org.chromium.ui.modelutil.PropertyModel.WritableObjectPropertyKey;

/** The properties associated with rendering AI suggestion views. */
public @interface AiSuggestionViewProperties {
    /** Whether suggestion is an AI suggestion. */
    static final WritableBooleanPropertyKey IS_AI_SUGGESTION = new WritableBooleanPropertyKey();

    /** Whether the AI suggestion is in loading state (showing blur effect). */
    static final WritableBooleanPropertyKey IS_LOADING = new WritableBooleanPropertyKey();

    /** The actual text content for the first line of text. */
    @VisibleForTesting
    static final WritableObjectPropertyKey<SuggestionSpannable> TEXT_LINE_1_TEXT =
            new WritableObjectPropertyKey<>();

    /** The actual text content for the second line of text. */
    @VisibleForTesting
    static final WritableObjectPropertyKey<SuggestionSpannable> TEXT_LINE_2_TEXT =
            new WritableObjectPropertyKey<>();

    static final PropertyKey[] ALL_UNIQUE_KEYS =
            new PropertyKey[] {
                IS_AI_SUGGESTION, IS_LOADING, TEXT_LINE_1_TEXT, TEXT_LINE_2_TEXT
            };

    static final PropertyKey[] ALL_KEYS =
            PropertyModel.concatKeys(ALL_UNIQUE_KEYS, BaseSuggestionViewProperties.ALL_KEYS);
}
