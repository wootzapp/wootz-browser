// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.transit;

import org.chromium.base.test.transit.ScrollableFacility.Item.Presence;

/**
 * A facility representing an item inside a {@link ScrollableFacility} shown on the screen.
 *
 * @param <HostStationT> the type of host {@link Station} this is scoped to.
 * @param <SelectReturnT> the return type of the |selectHandler|.
 */
public class ItemOnScreenFacility<HostStationT extends Station, SelectReturnT>
        extends Facility<HostStationT> {

    protected final ScrollableFacility<HostStationT>.Item<SelectReturnT> mItem;

    protected ItemOnScreenFacility(
            HostStationT station, ScrollableFacility<HostStationT>.Item<SelectReturnT> item) {
        super(station);
        mItem = item;
    }

    @Override
    public void declareElements(Elements.Builder elements) {
        elements.declareView(mItem.getViewElement());
    }

    /** Select the item and trigger its |selectHandler|. */
    public SelectReturnT select() {
        if (mItem.getPresence() == Presence.ABSENT) {
            throw new IllegalStateException("Cannot click on an absent item");
        }
        if (mItem.getPresence() == Presence.PRESENT_AND_DISABLED) {
            throw new IllegalStateException("Cannot click on a disabled item");
        }

        try {
            return mItem.getSelectHandler().call();
        } catch (Exception e) {
            throw TravelException.newTravelException("Select handler threw an exception:", e);
        }
    }
}
