"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.effect = effect;
/**
 * Run a function whenever dependencies change
 * @param effectFn - Side effect function to run
 * @param dependencies - The stores it depends on
 * @param runOnMount - Whether to run the effect function immediately or not
 * @example
 * ```ts
 * const foo = computed(() => bar.getValue() + baz.getValue(), [bar, baz]);
 * ```
 * @returns A function to stop listening to changes in the dependencies
 */
function effect(
// pass the values of the dependencies to the computation function
effectFn, 
// biome-ignore lint/suspicious/noExplicitAny: library function that accepts any store type
dependencies, runOnMount = true) {
    if (runOnMount) {
        effectFn();
    }
    // when any of the dependencies change, recompute the value and set it
    const unsubscribeList = dependencies.map((store) => {
        return store.subscribe(() => {
            effectFn();
        });
    });
    return () => {
        for (const fn of unsubscribeList) {
            fn();
        }
    };
}
//# sourceMappingURL=effect.js.map