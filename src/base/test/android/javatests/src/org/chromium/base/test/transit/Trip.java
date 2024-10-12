// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.transit;

import androidx.annotation.Nullable;

import org.chromium.base.Log;
import org.chromium.base.test.transit.ConditionWaiter.ConditionWait;
import org.chromium.base.test.transit.ConditionalState.Phase;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * A {@link Transition} into a {@link Station}, either from another {@link Station} or as an entry
 * point.
 */
class Trip extends Transition {
    private static final String TAG = "Transit";
    private final int mId;

    private final Station mOrigin;
    private final Station mDestination;

    private List<ConditionWait> mWaits;

    private static int sLastTripId;

    Trip(Station origin, Station destination, TransitionOptions options, Trigger trigger) {
        super(options, trigger);
        mOrigin = origin;
        mDestination = destination;
        mId = ++sLastTripId;
    }

    void transitionSync() {
        // TODO(crbug.com/333735412): Unify Trip#travelSyncInternal(), FacilityCheckIn#enterSync()
        // and FacilityCheckOut#exitSync().
        embark();
        Log.i(TAG, "Trip %d: Embarked at %s towards %s", mId, mOrigin, mDestination);

        if (mOptions.mTries == 1) {
            triggerTransition();
            Log.i(TAG, "Trip %d: Triggered transition, waiting to arrive at %s", mId, mDestination);
            waitUntilArrival();
        } else {
            for (int tryNumber = 1; tryNumber <= mOptions.mTries; tryNumber++) {
                try {
                    triggerTransition();
                    Log.i(
                            TAG,
                            "Trip %d: Triggered transition (try #%d/%d), waiting to arrive at %s",
                            mId,
                            tryNumber,
                            mOptions.mTries,
                            mDestination);
                    waitUntilArrival();
                    break;
                } catch (TravelException e) {
                    Log.w(TAG, "Try #%d failed", tryNumber, e);
                    if (tryNumber >= mOptions.mTries) {
                        throw e;
                    }
                }
            }
        }

        Log.i(TAG, "Trip %d: Arrived at %s", mId, mDestination);

        PublicTransitConfig.maybePauseAfterTransition(mDestination);
    }

    private void embark() {
        mOrigin.setStateTransitioningFrom();
        mDestination.setStateTransitioningTo();

        mWaits = calculateConditionWaits(mOrigin, mDestination, getTransitionConditions());
        ConditionWaiter.preCheck(mWaits, mOptions, mTrigger);
        for (ConditionWait wait : mWaits) {
            wait.getCondition().onStartMonitoring();
        }
    }

    private void waitUntilArrival() {
        // Throws CriteriaNotSatisfiedException if any conditions aren't met within the timeout and
        // prints the state of all conditions. The timeout can be reduced when explicitly looking
        // for flakiness due to tight timeouts.
        try {
            ConditionWaiter.waitFor(mWaits, mOptions);
        } catch (AssertionError e) {
            throw newTransitionException(e);
        }

        mOrigin.setStateFinished();
        mDestination.setStateActive();
        for (ConditionWait waits : mWaits) {
            waits.getCondition().onStopMonitoring();
        }

        TrafficControl.notifyActiveStationChanged(mDestination);
    }

    private static ArrayList<ConditionWait> calculateConditionWaits(
            Station origin, Station destination, List<Condition> transitionConditions) {
        ArrayList<ConditionWait> waits = new ArrayList<>();

        Elements originElements =
                origin.getElementsIncludingFacilitiesWithPhase(Phase.TRANSITIONING_FROM);
        Elements destinationElements =
                destination.getElementsIncludingFacilitiesWithPhase(Phase.TRANSITIONING_TO);

        // Create ENTER Conditions for Views that should appear and LogicalElements that should
        // be true.
        Set<String> destinationElementIds = new HashSet<>();
        for (ElementInState element : destinationElements.getElementsInState()) {
            destinationElementIds.add(element.getId());
            @Nullable Condition enterCondition = element.getEnterCondition();
            if (enterCondition != null) {
                waits.add(new ConditionWait(enterCondition, ConditionWaiter.ConditionOrigin.ENTER));
            }
        }

        // Add extra ENTER Conditions.
        for (Condition enterCondition : destinationElements.getOtherEnterConditions()) {
            waits.add(new ConditionWait(enterCondition, ConditionWaiter.ConditionOrigin.ENTER));
        }

        // Create EXIT Conditions for Views that should disappear and LogicalElements that should
        // be false.
        for (ElementInState element : originElements.getElementsInState()) {
            Condition exitCondition = element.getExitCondition(destinationElementIds);
            if (exitCondition != null) {
                waits.add(new ConditionWait(exitCondition, ConditionWaiter.ConditionOrigin.EXIT));
            }
        }

        // Add extra EXIT Conditions.
        for (Condition exitCondition : originElements.getOtherExitConditions()) {
            waits.add(new ConditionWait(exitCondition, ConditionWaiter.ConditionOrigin.EXIT));
        }

        // Add transition (TRSTN) conditions
        for (Condition condition : transitionConditions) {
            waits.add(new ConditionWait(condition, ConditionWaiter.ConditionOrigin.TRANSITION));
        }

        return waits;
    }

    @Override
    public String toDebugString() {
        return String.format("Trip from %s to %s", mOrigin, mDestination);
    }
}
