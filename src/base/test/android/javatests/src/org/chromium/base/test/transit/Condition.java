// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.transit;

import android.util.ArrayMap;

import androidx.annotation.Nullable;

import com.google.errorprone.annotations.FormatMethod;

import org.chromium.base.supplier.Supplier;
import org.chromium.base.test.transit.ConditionStatus.Status;

/**
 * A condition that needs to be fulfilled for a state transition to be considered done.
 *
 * <p>{@link ConditionWaiter} waits for multiple Conditions to be fulfilled. {@link
 * ConditionChecker} performs one-time checks for whether multiple Conditions are fulfilled.
 */
public abstract class Condition {
    private String mDescription;

    private boolean mIsRunOnUiThread;
    private ArrayMap<String, Supplier<?>> mDependentSuppliers;

    /**
     * @param isRunOnUiThread true if the Condition should be checked on the UI Thread, false if it
     *     should be checked on the Instrumentation Thread. Other hooks such as {@link
     *     #onStartMonitoring()} are always run on the Instrumentation Thread.
     */
    public Condition(boolean isRunOnUiThread) {
        mIsRunOnUiThread = isRunOnUiThread;
    }

    /**
     * Should check the condition, report its status (if useful) and return whether it is fulfilled.
     *
     * <p>Depending on #shouldRunOnUiThread(), called on the UI or the instrumentation thread.
     *
     * @return {@link ConditionStatus} stating whether the condition has been fulfilled and
     *     optionally more details about its state.
     */
    protected abstract ConditionStatus checkWithSuppliers() throws Exception;

    /**
     * @return a short description to be printed as part of a list of conditions. Use {@link
     *     #getDescription()} to get a description as it caches the description until {@link
     *     #rebuildDescription()} invalidates it.
     */
    public abstract String buildDescription();

    /**
     * Hook run right before the condition starts being checked. Used, for example, to get initial
     * callback counts and install observers.
     */
    public void onStartMonitoring() {}

    /**
     * Hook run right after the condition stops being checked. Used, for example, to uninstall
     * observers.
     */
    public void onStopMonitoring() {}

    /**
     * @return a short description to be printed as part of a list of conditions.
     */
    public String getDescription() {
        if (mDescription == null) {
            rebuildDescription();
        }
        return mDescription;
    }

    /**
     * Invalidates last description; the next time {@link #getDescription()}, it will get a new one
     * from {@link #buildDescription()}.
     */
    protected void rebuildDescription() {
        mDescription = buildDescription();
    }

    /**
     * @return true if the check is intended to be run on the UI Thread, false if it should be run
     *     on the instrumentation thread.
     */
    public boolean isRunOnUiThread() {
        return mIsRunOnUiThread;
    }

    /**
     * Declare a Supplier this Condition's check() depends on.
     *
     * <p>Call this from the constructor to delay check() to be called until |supplier| has a value.
     */
    protected <T> Supplier<T> dependOnSupplier(Supplier<T> supplier, String inputName) {
        if (mDependentSuppliers == null) {
            mDependentSuppliers = new ArrayMap<>();
        }
        mDependentSuppliers.put(inputName, supplier);
        return supplier;
    }

    /**
     * The method called to actually check the Condition, including checking dependencies of
     * check().
     */
    public final ConditionStatus check() throws Exception {
        // If any Supplier is missing a value, the Condition can't be checked yet.
        ConditionStatus status = checkDependentSuppliers();
        if (status != null) {
            return status;
        }

        // Call the subclass' checkWithSuppliers().
        return checkWithSuppliers();
    }

    private ConditionStatus checkDependentSuppliers() {
        if (mDependentSuppliers == null) {
            return null;
        }

        StringBuilder suppliersMissing = null;
        for (var kv : mDependentSuppliers.entrySet()) {
            Supplier<?> supplier = kv.getValue();
            if (!supplier.hasValue()) {
                if (suppliersMissing == null) {
                    suppliersMissing = new StringBuilder("waiting for suppliers of: ");
                } else {
                    suppliersMissing.append(", ");
                }
                String inputName = kv.getKey();
                suppliersMissing.append(inputName);
            }
        }

        if (suppliersMissing != null) {
            return awaiting(suppliersMissing.toString());
        }

        return null;
    }

    /** {@link #checkWithSuppliers()} should return this when a Condition is fulfilled. */
    public static ConditionStatus fulfilled() {
        return fulfilled(/* message= */ null);
    }

    /** {@link #fulfilled()} with more details to be logged as a short message. */
    public static ConditionStatus fulfilled(@Nullable String message) {
        return new ConditionStatus(Status.FULFILLED, message);
    }

    /** {@link #fulfilled()} with more details to be logged as a short message. */
    @FormatMethod
    public static ConditionStatus fulfilled(String message, Object... args) {
        return new ConditionStatus(Status.FULFILLED, String.format(message, args));
    }

    /** {@link #checkWithSuppliers()} should return this when a Condition is not fulfilled. */
    public static ConditionStatus notFulfilled() {
        return notFulfilled(/* message= */ null);
    }

    /** {@link #notFulfilled()} with more details to be logged as a short message. */
    public static ConditionStatus notFulfilled(@Nullable String message) {
        return new ConditionStatus(Status.NOT_FULFILLED, message);
    }

    /** {@link #notFulfilled()} with more details to be logged as a short message. */
    @FormatMethod
    public static ConditionStatus notFulfilled(String message, Object... args) {
        return new ConditionStatus(Status.NOT_FULFILLED, String.format(message, args));
    }

    /**
     * {@link #checkWithSuppliers()} should return this when an error happens while checking a
     * Condition.
     *
     * <p>A short message is required.
     *
     * <p>Throwing an error in check() has the same effect.
     */
    public static ConditionStatus error(@Nullable String message) {
        return new ConditionStatus(Status.ERROR, message);
    }

    /** {@link #error(String)} with format parameters. */
    @FormatMethod
    public static ConditionStatus error(String message, Object... args) {
        return new ConditionStatus(Status.ERROR, String.format(message, args));
    }

    /** {@link #checkWithSuppliers()} should return this as a convenience method. */
    public static ConditionStatus whether(boolean isFulfilled) {
        return isFulfilled ? fulfilled() : notFulfilled();
    }

    /** {@link #whether(boolean)} with more details to be logged as a short message. */
    public static ConditionStatus whether(boolean isFulfilled, @Nullable String message) {
        return isFulfilled ? fulfilled(message) : notFulfilled(message);
    }

    /** {@link #whether(boolean)} with more details to be logged as a short message. */
    @FormatMethod
    public static ConditionStatus whether(boolean isFulfilled, String message, Object... args) {
        return whether(isFulfilled, String.format(message, args));
    }

    /**
     * {@link #checkWithSuppliers()} should return this when it does not have information to check
     * the Condition yet.
     *
     * <p>It is considered not fulfilled for most purposes. The exception is that if the Condition
     * is used as a gate Condition, the gated Condition will not be checked, considered FULFILLED,
     * or considered NOT_FULFILLED until the gate resolves to FULFILLED or NOT_FULFILLED.
     *
     * @param message A short message stating what is being awaited for
     */
    public static ConditionStatus awaiting(@Nullable String message) {
        return new ConditionStatus(Status.AWAITING, message);
    }

    /** {@link #awaiting(String)} with format parameters. */
    @FormatMethod
    public static ConditionStatus awaiting(String message, Object... args) {
        return new ConditionStatus(Status.AWAITING, String.format(message, args));
    }

    /** {@link #checkWithSuppliers()} can return this as a convenience method. */
    public static ConditionStatus fulfilledOrAwaiting(
            boolean isFulfilled, @Nullable String message) {
        return isFulfilled ? fulfilled(message) : awaiting(message);
    }

    /**
     * {@link #fulfilledOrAwaiting(boolean, String)} with more details to be logged as a short
     * message.
     */
    @FormatMethod
    public static ConditionStatus fulfilledOrAwaiting(
            boolean isFulfilled, String message, Object... args) {
        return fulfilledOrAwaiting(isFulfilled, String.format(message, args));
    }
}
