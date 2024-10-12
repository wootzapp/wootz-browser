import { ExpressionNode, ExpressionTerm, FunctionNode, IdentNode, NumberNode, OperatorNode, Percentage, Unit } from './parsers';
export declare type Evaluatable<T> = Evaluator<T> | T;
/**
 * A NumberNodeSequence is a vector of NumberNodes with a specified
 * sequence of units.
 */
export declare type NumberNodeSequence<T extends Array<Unit>, U = never> = {
    [I in keyof T]: NumberNode & {
        unit: T[I] | U;
    };
};
export declare type Sparse<T> = {
    [I in keyof T]: null | T[I];
};
/**
 * Intrinsics describe the metadata required to do four things for any given
 * type of number-based CSS expression:
 *
 *  1. Establish the expected units of a final, evaluated result
 *  2. Provide a foundational value that percentages should scale against
 *  3. Describe the analog number values that correspond to various keywords
 *  4. Have an available concrete value to fallback to when needed
 *
 * Intrinsics must always specify a basis and the substitute values for the
 * keyword 'auto'.
 *
 * Intrinsics may optionally specify the substitute values for any additional
 * number of keywords.
 */
export interface Intrinsics<T extends Array<Unit> = []> {
    basis: NumberNodeSequence<T>;
    keywords: {
        auto: Sparse<NumberNodeSequence<T, Percentage>>;
        [index: string]: Sparse<NumberNodeSequence<T, Percentage>>;
    };
}
declare const $evaluate: unique symbol;
declare const $lastValue: unique symbol;
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
export declare abstract class Evaluator<T> {
    /**
     * An Evaluatable is a NumberNode or an Evaluator that evaluates a NumberNode
     * as the result of invoking its evaluate method. This is mainly used to
     * ensure that CSS function nodes are cast to the corresponding Evaluators
     * that will resolve the result of the function, but is also used to ensure
     * that a percentage nested at arbitrary depth in the expression will always
     * be evaluated against the correct basis.
     */
    static evaluatableFor(node: ExpressionTerm | Evaluator<NumberNode>, basis?: NumberNode): Evaluatable<NumberNode>;
    /**
     * If the input is an Evaluator, returns the result of evaluating it.
     * Otherwise, returns the input.
     *
     * This is a helper to aide in resolving a NumberNode without conditionally
     * checking if the Evaluatable is an Evaluator everywhere.
     */
    static evaluate<T extends NumberNode | IdentNode>(evaluatable: Evaluatable<T>): T;
    /**
     * If the input is an Evaluator, returns the value of its isConstant property.
     * Returns true for all other input values.
     */
    static isConstant<T>(evaluatable: Evaluatable<T>): boolean;
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
    static applyIntrinsics<T extends Array<Unit>>(evaluated: Array<any>, intrinsics: Intrinsics<T>): NumberNodeSequence<T>;
    /**
     * If true, the Evaluator will only evaluate its AST one time. If false, the
     * Evaluator will re-evaluate the AST each time that the public evaluate
     * method is invoked.
     */
    get isConstant(): boolean;
    protected [$lastValue]: T | null;
    /**
     * This method must be implemented by subclasses. Its implementation should be
     * the actual steps to evaluate the AST, and should return the evaluated
     * result.
     */
    protected abstract [$evaluate](): T;
    /**
     * Evaluate the Evaluator and return the result. If the Evaluator is constant,
     * the corresponding AST will only be evaluated once, and the result of
     * evaluating it the first time will be returned on all subsequent
     * evaluations.
     */
    evaluate(): T;
}
declare const $percentage: unique symbol;
declare const $basis: unique symbol;
/**
 * A PercentageEvaluator scales a given basis value by a given percentage value.
 * The evaluated result is always considered to be constant.
 */
export declare class PercentageEvaluator extends Evaluator<NumberNode> {
    protected [$percentage]: NumberNode<'%'>;
    protected [$basis]: NumberNode;
    constructor(percentage: NumberNode<'%'>, basis: NumberNode);
    get isConstant(): boolean;
    [$evaluate](): NumberNode<Unit | "%" | null>;
}
declare const $identNode: unique symbol;
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
export declare class EnvEvaluator extends Evaluator<NumberNode> {
    protected [$identNode]: IdentNode | null;
    constructor(envFunction: FunctionNode);
    get isConstant(): boolean;
    [$evaluate](): NumberNode;
}
declare const $evaluator: unique symbol;
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
export declare class CalcEvaluator extends Evaluator<NumberNode> {
    protected [$evaluator]: Evaluator<NumberNode> | null;
    constructor(calcFunction: FunctionNode, basis?: NumberNode);
    get isConstant(): boolean;
    [$evaluate](): NumberNode<Unit | "%" | null>;
}
declare const $operator: unique symbol;
declare const $left: unique symbol;
declare const $right: unique symbol;
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
export declare class OperatorEvaluator extends Evaluator<NumberNode> {
    protected [$operator]: OperatorNode;
    protected [$left]: Evaluatable<NumberNode>;
    protected [$right]: Evaluatable<NumberNode>;
    constructor(operator: OperatorNode, left: Evaluatable<NumberNode>, right: Evaluatable<NumberNode>);
    get isConstant(): boolean;
    [$evaluate](): NumberNode;
}
export declare type EvaluatedStyle<T extends Intrinsics<Array<Unit>>> = {
    [I in keyof T['basis']]: number;
} & Array<never>;
declare const $evaluatables: unique symbol;
declare const $intrinsics: unique symbol;
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
export declare class StyleEvaluator<T extends Intrinsics<Array<any>>> extends Evaluator<EvaluatedStyle<T>> {
    protected [$intrinsics]: T;
    protected [$evaluatables]: Array<Evaluatable<NumberNode | IdentNode>>;
    constructor(expressions: Array<ExpressionNode>, intrinsics: T);
    get isConstant(): boolean;
    [$evaluate](): EvaluatedStyle<T>;
}
export declare type SphericalIntrinsics = Intrinsics<['rad', 'rad', 'm']>;
export declare type Vector3Intrinsics = Intrinsics<['m', 'm', 'm']>;
export {};
