// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions.querytiles;

import android.content.Context;
import android.graphics.drawable.BitmapDrawable;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Px;

import org.chromium.chrome.browser.omnibox.OmniboxFeatures;
import org.chromium.chrome.browser.omnibox.R;
import org.chromium.chrome.browser.omnibox.styles.OmniboxImageSupplier;
import org.chromium.chrome.browser.omnibox.suggestions.SuggestionHost;
import org.chromium.chrome.browser.omnibox.suggestions.base.DynamicSpacingRecyclerViewItemDecoration;
import org.chromium.chrome.browser.omnibox.suggestions.carousel.BaseCarouselSuggestionItemViewBuilder;
import org.chromium.chrome.browser.omnibox.suggestions.carousel.BaseCarouselSuggestionProcessor;
import org.chromium.chrome.browser.omnibox.suggestions.carousel.BaseCarouselSuggestionViewProperties;
import org.chromium.components.omnibox.AutocompleteMatch;
import org.chromium.components.omnibox.OmniboxSuggestionType;
import org.chromium.components.omnibox.suggestions.OmniboxSuggestionUiType;
import org.chromium.ui.modelutil.MVCListAdapter.ListItem;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.ArrayList;
import java.util.List;

/** SuggestionProcessor for Query Tiles. */
public class QueryTilesProcessor extends BaseCarouselSuggestionProcessor {
    private static final float LAST_ELEMENT_MIN_EXPOSURE_FRACTION = 0.3f;
    private static final float LAST_ELEMENT_MAX_EXPOSURE_FRACTION = 0.7f;
    private final @NonNull SuggestionHost mSuggestionHost;
    private final @Nullable OmniboxImageSupplier mImageSupplier;
    private final @Px int mCarouselItemViewWidth;
    private final @Px int mCarouselItemViewHeight;
    private final @Px int mInitialSpacing;
    private final @Px int mElementSpacing;

    /**
     * Constructor.
     *
     * @param context An Android context.
     * @param host SuggestionHost receiving notifications about user actions.
     * @param imageSupplier Class retrieving favicons for the MV Tiles.
     */
    public QueryTilesProcessor(
            @NonNull Context context,
            @NonNull SuggestionHost host,
            @Nullable OmniboxImageSupplier imageSupplier) {
        super(context);
        mSuggestionHost = host;
        mImageSupplier = imageSupplier;
        mCarouselItemViewWidth =
                mContext.getResources().getDimensionPixelSize(R.dimen.query_tile_view_width);
        mCarouselItemViewHeight =
                mContext.getResources().getDimensionPixelSize(R.dimen.query_tile_view_height);

        mInitialSpacing =
                context.getResources()
                        .getDimensionPixelSize(
                                R.dimen.omnibox_query_tiles_carousel_horizontal_leadin);
        mElementSpacing =
                context.getResources()
                        .getDimensionPixelSize(
                                R.dimen.omnibox_query_tiles_carousel_horizontal_space);
    }

    @Override
    public boolean doesProcessSuggestion(AutocompleteMatch match, int matchIndex) {
        // This component only processes QueryTiles represented as a Carousel.
        // QueryTiles that are meant to show as a list are processed by the default processor: the
        // BasicSuggestionProcessor.
        return match.getType() == OmniboxSuggestionType.TILE_SUGGESTION
                && OmniboxFeatures.QUERY_TILES_SHOW_AS_CAROUSEL.getValue();
    }

    @Override
    public int getViewTypeId() {
        return OmniboxSuggestionUiType.QUERY_TILES;
    }

    @Override
    public PropertyModel createModel() {
        var padding =
                mContext.getResources()
                        .getDimensionPixelSize(
                                R.dimen.omnibox_query_tiles_carousel_vertical_padding);
        return new PropertyModel.Builder(BaseCarouselSuggestionViewProperties.ALL_KEYS)
                .with(BaseCarouselSuggestionViewProperties.TILES, new ArrayList<>())
                .with(
                        BaseCarouselSuggestionViewProperties.CONTENT_DESCRIPTION,
                        mContext.getResources()
                                .getString(R.string.accessibility_omnibox_query_tiles_list))
                .with(BaseCarouselSuggestionViewProperties.TOP_PADDING, padding)
                .with(BaseCarouselSuggestionViewProperties.BOTTOM_PADDING, padding)
                .with(BaseCarouselSuggestionViewProperties.APPLY_BACKGROUND, true)
                .with(
                        BaseCarouselSuggestionViewProperties.ITEM_DECORATION,
                        new DynamicSpacingRecyclerViewItemDecoration(
                                mInitialSpacing,
                                mElementSpacing,
                                mCarouselItemViewWidth,
                                LAST_ELEMENT_MIN_EXPOSURE_FRACTION,
                                LAST_ELEMENT_MAX_EXPOSURE_FRACTION))
                .build();
    }

    @Override
    public int getCarouselItemViewHeight() {
        return mCarouselItemViewHeight;
    }

    @Override
    public void populateModel(AutocompleteMatch match, PropertyModel model, int matchIndex) {
        super.populateModel(match, model, matchIndex);

        List<ListItem> tileList = model.get(BaseCarouselSuggestionViewProperties.TILES);
        var tileModel =
                new PropertyModel.Builder(QueryTileViewProperties.ALL_UNIQUE_KEYS)
                        .with(QueryTileViewProperties.TITLE, match.getDisplayText())
                        .with(
                                QueryTileViewProperties.ON_FOCUS_VIA_SELECTION,
                                () ->
                                        mSuggestionHost.setOmniboxEditingText(
                                                match.getFillIntoEdit()))
                        .with(
                                QueryTileViewProperties.ON_CLICK,
                                v ->
                                        mSuggestionHost.onSuggestionClicked(
                                                match, matchIndex, match.getUrl()))
                        .build();
        tileList.add(
                new ListItem(BaseCarouselSuggestionItemViewBuilder.ViewType.QUERY_TILE, tileModel));

        if (mImageSupplier != null && match.getImageUrl().isValid()) {
            mImageSupplier.fetchImage(
                    match.getImageUrl(),
                    image ->
                            tileModel.set(
                                    QueryTileViewProperties.IMAGE,
                                    new BitmapDrawable(mContext.getResources(), image)));
        }
    }
}
