export declare type Operator = '+' | '-' | '*' | '/';
export declare type Unit = 'm' | 'cm' | 'mm' | 'rad' | 'deg';
export declare type Percentage = '%';
export declare type ExpressionTerm = IdentNode | HexNode | NumberNode | OperatorNode | FunctionNode;
export interface IdentNode {
    type: 'ident';
    value: string;
}
export interface HexNode {
    type: 'hex';
    value: string;
}
export interface NumberNode<U = Unit | Percentage | null> {
    type: 'number';
    number: number;
    unit: U;
}
export declare const numberNode: <T extends Unit | "%" | null>(value: number, unit: T) => NumberNode<T>;
export interface OperatorNode {
    type: 'operator';
    value: Operator;
}
export interface FunctionNode {
    type: 'function';
    name: IdentNode;
    arguments: Array<ExpressionNode>;
}
export interface ExpressionNode {
    type: 'expression';
    terms: Array<ExpressionTerm>;
}
export declare type ASTNode = IdentNode | HexNode | NumberNode | OperatorNode | FunctionNode | ExpressionNode;
/**
 * Given a string representing a comma-separated set of CSS-like expressions,
 * parses and returns an array of ASTs that correspond to those expressions.
 *
 * Currently supported syntax includes:
 *
 *  - functions (top-level and nested)
 *  - calc() arithmetic operators
 *  - numbers with units
 *  - hexadecimal-encoded colors in 3, 6 or 8 digit form
 *  - idents
 *
 * All syntax is intended to match the parsing rules and semantics of the actual
 * CSS spec as closely as possible.
 *
 * @see https://www.w3.org/TR/CSS2/
 * @see https://www.w3.org/TR/css-values-3/
 */
export declare const parseExpressions: (inputString: string) => Array<ExpressionNode>;
export declare type ASTWalkerCallback<T> = (node: T) => void;
declare const $visitedTypes: unique symbol;
/**
 * An ASTWalker walks an array of ASTs such as the type produced by
 * parseExpressions and invokes a callback for a configured set of nodes that
 * the user wishes to "visit" during the walk.
 */
export declare class ASTWalker<T extends ASTNode> {
    protected [$visitedTypes]: Array<string>;
    constructor(visitedTypes: Array<string>);
    /**
     * Walk the given set of ASTs, and invoke the provided callback for nodes that
     * match the filtered set that the ASTWalker was constructed with.
     */
    walk(ast: Array<ExpressionNode>, callback: ASTWalkerCallback<T>): void;
}
export declare const ZERO: NumberNode;
export {};
