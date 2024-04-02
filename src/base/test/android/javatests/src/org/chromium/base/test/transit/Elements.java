// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.transit;

import java.util.ArrayList;
import java.util.List;

/**
 * The elements that define a {@link ConditionalState}.
 *
 * <pre>
 * - An ACTIVE ConditionalState is considered to have these elements available.
 * - The presence of each element is an enter condition for the ConditionalState.
 * - The absence of each element is an exit condition for the ConditionalState (except for unowned
 *     elements).
 * </pre>
 */
public class Elements {

    /** If passed as |id|, the description is considered the id. */
    public static final String DESCRIPTION_AS_ID = "__DESCRIPTION_AS_ID";

    static final Elements EMPTY = new Elements();

    private ArrayList<ElementInState> mElementsInState = new ArrayList<>();
    private ArrayList<Condition> mOtherEnterConditions = new ArrayList<>();
    private ArrayList<Condition> mOtherExitConditions = new ArrayList<>();

    /** Private constructor, instantiated by {@link Builder#build()}. */
    private Elements() {}

    public static Builder newBuilder() {
        return new Builder(new Elements());
    }

    List<ElementInState> getElementsInState() {
        return mElementsInState;
    }

    List<Condition> getOtherEnterConditions() {
        return mOtherEnterConditions;
    }

    List<Condition> getOtherExitConditions() {
        return mOtherExitConditions;
    }

    /**
     * Builder for {@link Elements}.
     *
     * <p>Passed to {@link ConditionalState#declareElements(Builder)}, which must declare the
     * ConditionalState's elements by calling the declare___() methods.
     */
    public static class Builder {

        private Elements mElements;

        /** Instantiate by calling {@link Elements#newBuilder()}. */
        private Builder(Elements elements) {
            mElements = elements;
        }

        /** Declare as an element a View that matches |viewMatcher|. */
        public Builder declareView(ViewElement viewElement) {
            mElements.mElementsInState.add(new ViewElementInState(viewElement, /* gate= */ null));
            return this;
        }

        /**
         * Conditional version of {@link #declareView(ViewElement)}.
         *
         * <p>The element is only expected if |gate| returns true.
         */
        public Builder declareViewIf(ViewElement viewElement, Condition gate) {
            mElements.mElementsInState.add(new ViewElementInState(viewElement, gate));
            return this;
        }

        /**
         * Declare as an element a logical check that must return true when and as long as the
         * Station is ACTIVE.
         *
         * <p>Differs from {@link #declareEnterCondition(Condition)} in that shared-scope
         * LogicalElements do not generate exit Conditions when going to another ConditionalState
         * with the same LogicalElement.
         */
        public Builder declareLogicalElement(LogicalElement logicalElement) {
            mElements.mElementsInState.add(logicalElement);
            return this;
        }

        /**
         * Declare as an element a generic enter Condition. It must be true for a transition into
         * this ConditionalState to be complete.
         *
         * <p>No promises are made that the Condition is true as long as the ConditionalState is
         * ACTIVE. For these cases, use {@link LogicalElement}.
         *
         * <p>Further, no promises are made that the Condition is false after exiting the State. Use
         * a scoped {@link LogicalElement} in this case.
         */
        public Builder declareEnterCondition(Condition condition) {
            mElements.mOtherEnterConditions.add(condition);
            return this;
        }

        /**
         * Declare as an element a generic exit Condition. It must be true for a transition out of
         * this ConditionalState to be complete.
         *
         * <p>No promises are made that the Condition is false as long as the ConditionalState is
         * ACTIVE. For these cases, use a scoped {@link LogicalElement}.
         */
        public Builder declareExitCondition(Condition condition) {
            mElements.mOtherExitConditions.add(condition);
            return this;
        }

        void addAll(Elements otherElements) {
            mElements.mElementsInState.addAll(otherElements.mElementsInState);
            mElements.mOtherEnterConditions.addAll(otherElements.mOtherEnterConditions);
            mElements.mOtherExitConditions.addAll(otherElements.mOtherExitConditions);
        }

        /**
         * Instantiates the {@link Elements} of a given |conditionalState| after they were declared
         * by calling the Builder's declare___() methods.
         */
        Elements build() {
            Elements elements = mElements;
            mElements = null;
            return elements;
        }
    }

}
