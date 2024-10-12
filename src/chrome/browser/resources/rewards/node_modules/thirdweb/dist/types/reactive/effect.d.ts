import type { ReadonlyStore } from "./computedStore.js";
import type { Store } from "./store.js";
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
export declare function effect<T>(effectFn: () => T, dependencies: (Store<any> | ReadonlyStore<any>)[], runOnMount?: boolean): () => void;
//# sourceMappingURL=effect.d.ts.map