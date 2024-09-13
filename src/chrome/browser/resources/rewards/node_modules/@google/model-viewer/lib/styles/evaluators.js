/* @license
 * Copyright 2019 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var _a, _b, _c;
import { normalizeUnit } from './conversions';
import { numberNode, ZERO } from './parsers';
const $evaluate = Symbol('evaluate');
const $lastValue = Symbol('lastValue');
/**
 * An Evaluator is used to derive a computed style from part (or all) of a CSS
 * expression AST. This construct is particularly useful for complex ASTs
 * containing function calls such as calc, var and env. Such styles could be
 * costly to re-evaluate on every frame (and in some cases we may try to do
 * that). The Evaluator construct allows us to mark sub-trees of the AST as
 * constant, so that only the dynamic parts are re-evaluated. It also separates
 * one-time AST preparation work from work that necessarily has to happen upon
 * each evaluation.
 */
export class Evaluator {
    constructor() {
        this[_a] = null;
    }
    /**
     * An Evaluatable is a NumberNode or an Evaluator that evaluates a NumberNode
     * as the result of invoking its evaluate method. This is mainly used to
     * ensure that CSS function nodes are cast to the corresponding Evaluators
     * that will resolve the result of the function, but is also used to ensure
     * that a percentage nested at arbitrary depth in the expression will always
     * be evaluated against the correct basis.
     */
    static evaluatableFor(node, basis = ZERO) {
        if (node instanceof Evaluator) {
            return node;
        }
        if (node.type === 'number') {
            if (node.unit === '%') {
                return new PercentageEvaluator(node, basis);
            }
            return node;
        }
        switch (node.name.value) {
            case 'calc':
                return new CalcEvaluator(node, basis);
            case 'env':
                return new EnvEvaluator(node);
        }
        return ZERO;
    }
    /**
     * If the input is an Evaluator, returns the result of evaluating it.
     * Otherwise, returns the input.
     *
     * This is a helper to aide in resolving a NumberNode without conditionally
     * checking if the Evaluatable is an Evaluator everywhere.
     */
    static evaluate(evaluatable) {
        if (evaluatable instanceof Evaluator) {
            return evaluatable.evaluate();
        }
        return evaluatable;
    }
    /**
     * If the input is an Evaluator, returns the value of its isConstant property.
     * Returns true for all other input values.
     */
    static isConstant(evaluatable) {
        if (evaluatable instanceof Evaluator) {
            return evaluatable.isConstant;
        }
        return true;
    }
    /**
     * This method applies a set of structured intrinsic metadata to an evaluated
     * result from a parsed CSS-like string of expressions. Intrinsics provide
     * sufficient metadata (e.g., basis values, analogs for keywords) such that
     * omitted values in the input string can be backfilled, and keywords can be
     * converted to concrete numbers.
     *
     * The result of applying intrinsics is a tuple of NumberNode values whose
     * units match the units used by the basis of the intrinsics.
     *
     * The following is a high-level description of how intrinsics are applied:
     *
     *  1. Determine the value of 'auto' for the current term
     *  2. If there is no corresponding input value for this term, substitute the
     *     'auto' value.
     *  3. If the term is an IdentNode, treat it as a keyword and perform the
     *     appropriate substitution.
     *  4. If the term is still null, fallback to the 'auto' value
     *  5. If the term is a percentage, apply it to the basis and return that
     *     value
     *  6. Normalize the unit of the term
     *  7. If the term's unit does not match the basis unit, return the basis
     *     value
     *  8. Return the term as is
     */
    static applyIntrinsics(evaluated, intrinsics) {
        const { basis, keywords } = intrinsics;
        const { auto } = keywords;
        return basis.map((basisNode, index) => {
            // Use an auto value if we have it, otherwise the auto value is the basis:
            const autoSubstituteNode = auto[index] == null ? basisNode : auto[index];
            // If the evaluated nodes do not have a node at the current
            // index, fallback to the "auto" substitute right away:
            let evaluatedNode = evaluated[index] ? evaluated[index] : autoSubstituteNode;
            // Any ident node is considered a keyword:
            if (evaluatedNode.type === 'ident') {
                const keyword = evaluatedNode.value;
                // Substitute any keywords for concrete values first:
                if (keyword in keywords) {
                    evaluatedNode = keywords[keyword][index];
                }
            }
            // If we don't have a NumberNode at this point, fall back to whatever
            // is specified for auto:
            if (evaluatedNode == null || evaluatedNode.type === 'ident') {
                evaluatedNode = autoSubstituteNode;
            }
            // For percentages, we always apply the percentage to the basis value:
            if (evaluatedNode.unit === '%') {
                return numberNode(evaluatedNode.number / 100 * basisNode.number, basisNode.unit);
            }
            // Otherwise, normalize whatever we have:
            evaluatedNode = normalizeUnit(evaluatedNode, basisNode);
            // If the normalized units do not match, return the basis as a fallback:
            if (evaluatedNode.unit !== basisNode.unit) {
                return basisNode;
            }
            // Finally, return the evaluated node with intrinsics applied:
            return evaluatedNode;
        });
    }
    /**
     * If true, the Evaluator will only evaluate its AST one time. If false, the
     * Evaluator will re-evaluate the AST each time that the public evaluate
     * method is invoked.
     */
    get isConstant() {
        return false;
    }
    /**
     * Evaluate the Evaluator and return the result. If the Evaluator is constant,
     * the corresponding AST will only be evaluated once, and the result of
     * evaluating it the first time will be returned on all subsequent
     * evaluations.
     */
    evaluate() {
        if (!this.isConstant || this[$lastValue] == null) {
            this[$lastValue] = this[$evaluate]();
        }
        return this[$lastValue];
    }
}
_a = $lastValue;
const $percentage = Symbol('percentage');
const $basis = Symbol('basis');
/**
 * A PercentageEvaluator scales a given basis value by a given percentage value.
 * The evaluated result is always considered to be constant.
 */
export class PercentageEvaluator extends Evaluator {
    constructor(percentage, basis) {
        super();
        this[$percentage] = percentage;
        this[$basis] = basis;
    }
    get isConstant() {
        return true;
    }
    [$evaluate]() {
        return numberNode(this[$percentage].number / 100 * this[$basis].number, this[$basis].unit);
    }
}
const $identNode = Symbol('identNode');
/**
 * Evaluator for CSS-like env() functions. Currently, only one environment
 * variable is accepted as an argument for such functions: window-scroll-y.
 *
 * The env() Evaluator is explicitly dynamic because it always refers to
 * external state that changes as the user scrolls, so it should always be
 * re-evaluated to ensure we get the most recent value.
 *
 * Some important notes about this feature include:
 *
 *  - There is no such thing as a "window-scroll-y" CSS environment variable in
 *    any stable browser at the time that this comment is being written.
 *  - The actual CSS env() function accepts a second argument as a fallback for
 *    the case that the specified first argument isn't set; our syntax does not
 *    support this second argument.
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/CSS/env
 */
export class EnvEvaluator extends Evaluator {
    constructor(envFunction) {
        super();
        this[_b] = null;
        const identNode = envFunction.arguments.length ? envFunction.arguments[0].terms[0] : null;
        if (identNode != null && identNode.type === 'ident') {
            this[$identNode] = identNode;
        }
    }
    get isConstant() {
        return false;
    }
    ;
    [(_b = $identNode, $evaluate)]() {
        if (this[$identNode] != null) {
            switch (this[$identNode].value) {
                case 'window-scroll-y':
                    const verticalScrollPosition = window.pageYOffset;
                    const verticalScrollMax = Math.max(document.body.scrollHeight, document.body.offsetHeight, document.documentElement.clientHeight, document.documentElement.scrollHeight, document.documentElement.offsetHeight);
                    const scrollY = verticalScrollPosition /
                        (verticalScrollMax - window.innerHeight) ||
                        0;
                    return { type: 'number', number: scrollY, unit: null };
            }
        }
        return ZERO;
    }
}
const IS_MULTIPLICATION_RE = /[\*\/]/;
const $evaluator = Symbol('evaluator');
/**
 * Evaluator for CSS-like calc() functions. Our implementation of calc()
 * evaluation currently support nested function calls, an unlimited number of
 * terms, and all four algebraic operators (+, -, * and /).
 *
 * The Evaluator is marked as constant unless the calc expression contains an
 * internal env expression at any depth, in which case it will be marked as
 * dynamic.
 *
 * @see https://www.w3.org/TR/css-values-3/#calc-syntax
 * @see https://developer.mozilla.org/en-US/docs/Web/CSS/calc
 */
export class CalcEvaluator extends Evaluator {
    constructor(calcFunction, basis = ZERO) {
        super();
        this[_c] = null;
        if (calcFunction.arguments.length !== 1) {
            return;
        }
        const terms = calcFunction.arguments[0].terms.slice();
        const secondOrderTerms = [];
        while (terms.length) {
            const term = terms.shift();
            if (secondOrderTerms.length > 0) {
                const previousTerm = secondOrderTerms[secondOrderTerms.length - 1];
                if (previousTerm.type === 'operator' &&
                    IS_MULTIPLICATION_RE.test(previousTerm.value)) {
                    const operator = secondOrderTerms.pop();
                    const leftValue = secondOrderTerms.pop();
                    if (leftValue == null) {
                        return;
                    }
                    secondOrderTerms.push(new OperatorEvaluator(operator, Evaluator.evaluatableFor(leftValue, basis), Evaluator.evaluatableFor(term, basis)));
                    continue;
                }
            }
            secondOrderTerms.push(term.type === 'operator' ? term :
                Evaluator.evaluatableFor(term, basis));
        }
        while (secondOrderTerms.length > 2) {
            const [left, operator, right] = secondOrderTerms.splice(0, 3);
            if (operator.type !== 'operator') {
                return;
            }
            secondOrderTerms.unshift(new OperatorEvaluator(operator, Evaluator.evaluatableFor(left, basis), Evaluator.evaluatableFor(right, basis)));
        }
        // There should only be one combined evaluator at this point:
        if (secondOrderTerms.length === 1) {
            this[$evaluator] = secondOrderTerms[0];
        }
    }
    get isConstant() {
        return this[$evaluator] == null || Evaluator.isConstant(this[$evaluator]);
    }
    [(_c = $evaluator, $evaluate)]() {
        return this[$evaluator] != null ? Evaluator.evaluate(this[$evaluator]) :
            ZERO;
    }
}
const $operator = Symbol('operator');
const $left = Symbol('left');
const $right = Symbol('right');
/**
 * An Evaluator for the operators found inside CSS calc() functions.
 * The evaluator accepts an operator and left/right operands. The operands can
 * be any valid expression term typically allowed inside a CSS calc function.
 *
 * As detail of this implementation, the only supported unit types are angles
 * expressed as radians or degrees, and lengths expressed as meters, centimeters
 * or millimeters.
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/CSS/calc
 */
export class OperatorEvaluator extends Evaluator {
    constructor(operator, left, right) {
        super();
        this[$operator] = operator;
        this[$left] = left;
        this[$right] = right;
    }
    get isConstant() {
        return Evaluator.isConstant(this[$left]) &&
            Evaluator.isConstant(this[$right]);
    }
    [$evaluate]() {
        const leftNode = normalizeUnit(Evaluator.evaluate(this[$left]));
        const rightNode = normalizeUnit(Evaluator.evaluate(this[$right]));
        const { number: leftValue, unit: leftUnit } = leftNode;
        const { number: rightValue, unit: rightUnit } = rightNode;
        // Disallow operations for mismatched normalized units e.g., m and rad:
        if (rightUnit != null && leftUnit != null && rightUnit != leftUnit) {
            return ZERO;
        }
        // NOTE(cdata): rules for calc type checking are defined here
        // https://drafts.csswg.org/css-values-3/#calc-type-checking
        // This is a simplification and may not hold up once we begin to support
        // additional unit types:
        const unit = leftUnit || rightUnit;
        let value;
        switch (this[$operator].value) {
            case '+':
                value = leftValue + rightValue;
                break;
            case '-':
                value = leftValue - rightValue;
                break;
            case '/':
                value = leftValue / rightValue;
                break;
            case '*':
                value = leftValue * rightValue;
                break;
            default:
                return ZERO;
        }
        return { type: 'number', number: value, unit };
    }
}
const $evaluatables = Symbol('evaluatables');
const $intrinsics = Symbol('intrinsics');
/**
 * A VectorEvaluator evaluates a series of numeric terms that usually represent
 * a data structure such as a multi-dimensional vector or a spherical
 *
 * The form of the evaluator's result is determined by the Intrinsics that are
 * given to it when it is constructed. For example, spherical intrinsics would
 * establish two angle terms and a length term, so the result of evaluating the
 * evaluator that is configured with spherical intrinsics is a three element
 * array where the first two elements represent angles in radians and the third
 * element representing a length in meters.
 */
export class StyleEvaluator extends Evaluator {
    constructor(expressions, intrinsics) {
        super();
        this[$intrinsics] = intrinsics;
        const firstExpression = expressions[0];
        const terms = firstExpression != null ? firstExpression.terms : [];
        this[$evaluatables] =
            intrinsics.basis.map((basisNode, index) => {
                const term = terms[index];
                if (term == null) {
                    return { type: 'ident', value: 'auto' };
                }
                if (term.type === 'ident') {
                    return term;
                }
                return Evaluator.evaluatableFor(term, basisNode);
            });
    }
    get isConstant() {
        for (const evaluatable of this[$evaluatables]) {
            if (!Evaluator.isConstant(evaluatable)) {
                return false;
            }
        }
        return true;
    }
    [$evaluate]() {
        const evaluated = this[$evaluatables].map(evaluatable => Evaluator.evaluate(evaluatable));
        return Evaluator.applyIntrinsics(evaluated, this[$intrinsics])
            .map(numberNode => numberNode.number);
    }
}
//# sourceMappingURL=evaluators.js.map