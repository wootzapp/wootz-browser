// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.transit;

import androidx.annotation.Nullable;

import org.chromium.base.Log;
import org.chromium.base.test.transit.ConditionWaiter.ConditionWait;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/** A {@link Transition} out of a {@link Facility}. */
class FacilityCheckOut extends Transition {
    private static final String TAG = "Transit";

    private Facility mFacility;
    private List<ConditionWait> mWaits;

    /**
     * Constructor. FacilityCheckOut is instantiated to leave a {@link Facility}.
     *
     * @param facility the {@link Facility} to leave.
     * @param options the {@link TransitionOptions}.
     * @param trigger the action that triggers the transition out of the facility. e.g. clicking a
     *     View.
     */
    FacilityCheckOut(Facility facility, TransitionOptions options, @Nullable Trigger trigger) {
        super(options, trigger);
        mFacility = facility;
    }

    void transitionSync() {
        // TODO(crbug.com/333735412): Unify Trip#travelSyncInternal(), FacilityCheckIn#enterSync()
        // and FacilityCheckOut#exitSync().
        onBeforeTransition();
        mWaits = createWaits();
        ConditionWaiter.preCheck(mWaits, mOptions, mTrigger);
        for (ConditionWait wait : mWaits) {
            wait.getCondition().onStartMonitoring();
        }

        if (mOptions.mTries == 1) {
            triggerTransition();
            Log.i(TAG, "Triggered transition, waiting to exit %s", mFacility);
            waitUntilExit(mWaits);
        } else {
            for (int tryNumber = 1; tryNumber <= mOptions.mTries; tryNumber++) {
                try {
                    triggerTransition();
                    Log.i(
                            TAG,
                            "Triggered transition (try #%d/%d), waiting to exit %s",
                            tryNumber,
                            mOptions.mTries,
                            mFacility);
                    waitUntilExit(mWaits);
                    break;
                } catch (TravelException e) {
                    Log.w(TAG, "Try #%d failed", tryNumber, e);
                    if (tryNumber >= mOptions.mTries) {
                        throw e;
                    }
                }
            }
        }

        onAfterTransition();
        PublicTransitConfig.maybePauseAfterTransition(mFacility);
    }

    private void onBeforeTransition() {
        mFacility.setStateTransitioningFrom();
        Log.i(TAG, "Will exit %s", mFacility);
    }

    @Override
    protected void triggerTransition() {
        super.triggerTransition();
        Log.i(TAG, "Triggered exit from %s", mFacility);
    }

    @Override
    public String toDebugString() {
        return "FacilityCheckOut for " + mFacility;
    }

    private List<ConditionWait> createWaits() {
        ArrayList<ConditionWait> waits = new ArrayList<>();
        for (ElementInState element : mFacility.getElements().getElementsInState()) {
            Condition exitCondition = element.getExitCondition(Collections.EMPTY_SET);
            if (exitCondition != null) {
                waits.add(new ConditionWait(exitCondition, ConditionWaiter.ConditionOrigin.EXIT));
            }
        }

        for (Condition exitCondition : mFacility.getElements().getOtherExitConditions()) {
            waits.add(new ConditionWait(exitCondition, ConditionWaiter.ConditionOrigin.EXIT));
        }

        for (Condition condition : getTransitionConditions()) {
            waits.add(new ConditionWait(condition, ConditionWaiter.ConditionOrigin.TRANSITION));
        }
        return waits;
    }

    private void waitUntilExit(List<ConditionWait> transitionConditions) {
        try {
            ConditionWaiter.waitFor(transitionConditions, mOptions);
        } catch (AssertionError e) {
            throw newTransitionException(e);
        }
    }

    private void onAfterTransition() {
        mFacility.setStateFinished();
        for (ConditionWait wait : mWaits) {
            wait.getCondition().onStopMonitoring();
        }
        Log.i(TAG, "Exited %s", mFacility);
    }
}
