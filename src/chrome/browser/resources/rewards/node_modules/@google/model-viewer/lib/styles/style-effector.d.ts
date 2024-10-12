import { ASTWalker, ExpressionNode, FunctionNode } from './parsers';
interface AnyObserver {
    observe(): void;
    disconnect(): void;
}
export declare type EnvironmentState = 'window-scroll';
export declare type StyleEffectorCallback = (record: EnvironmentChangeRecord) => void;
export interface EnvironmentChangeRecord {
    relatedState: EnvironmentState;
}
declare type EnvironmentDependencies = {
    [key in EnvironmentState]?: AnyObserver;
};
declare const $computeStyleCallback: unique symbol;
declare const $astWalker: unique symbol;
declare const $dependencies: unique symbol;
declare const $onScroll: unique symbol;
/**
 * The StyleEffector is configured with a callback that will be invoked at the
 * optimal time that some array of CSS expression ASTs ought to be evaluated.
 *
 * For example, our CSS-like expression syntax supports usage of the env()
 * function to incorporate the current top-level scroll position into a CSS
 * expression: env(window-scroll-y).
 *
 * This "environment variable" will change dynamically as the user scrolls the
 * page. If an AST contains such a usage of env(), we would have to evaluate the
 * AST on every frame in order to be sure that the computed style stays up to
 * date.
 *
 * The StyleEffector spares us from evaluating the expressions on every frame by
 * correlating specific parts of an AST with observers of the external effects
 * that they refer to (if any). So, if the AST contains env(window-scroll-y),
 * the StyleEffector manages the lifetime of a global scroll event listener and
 * notifies the user at the optimal time to evaluate the computed style.
 */
export declare class StyleEffector {
    protected [$dependencies]: EnvironmentDependencies;
    protected [$computeStyleCallback]: StyleEffectorCallback;
    protected [$astWalker]: ASTWalker<FunctionNode>;
    constructor(callback: StyleEffectorCallback);
    /**
     * Sets the expressions that govern when the StyleEffector callback will be
     * invoked.
     */
    observeEffectsFor(ast: Array<ExpressionNode>): void;
    /**
     * Disposes of the StyleEffector by disconnecting all observers of external
     * effects.
     */
    dispose(): void;
    protected [$onScroll]: () => void;
}
export {};
