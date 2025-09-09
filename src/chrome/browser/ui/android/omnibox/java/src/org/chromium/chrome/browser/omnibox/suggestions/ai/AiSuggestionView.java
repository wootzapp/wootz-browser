// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions.ai;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import org.chromium.chrome.browser.omnibox.R;
import org.chromium.chrome.browser.omnibox.suggestions.base.BaseSuggestionView;

/** A view for displaying AI suggestions in the omnibox. */
public class AiSuggestionView extends BaseSuggestionView<View> {
    /**
     * Constructor for inflating from XML.
     *
     * @param context The context the view is running in.
     * @param attrs The attributes of the XML tag that is inflating the view.
     */
    public AiSuggestionView(Context context, AttributeSet attrs) {
        super(context, R.layout.omnibox_ai_suggestion);
    }

    /**
     * Constructor for programmatic creation.
     *
     * @param context The context the view is running in.
     */
    public AiSuggestionView(Context context) {
        super(context, R.layout.omnibox_ai_suggestion);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        // Any additional initialization for AI suggestions can go here
    }
}
