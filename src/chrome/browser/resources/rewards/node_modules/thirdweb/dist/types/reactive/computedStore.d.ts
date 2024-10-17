import type { Store } from "./store.js";
export type ReadonlyStore<T> = {
    getValue(): T;
    subscribe(listener: () => void): () => void;
};
/**
 * Create a readonly store whose value is computed from other stores
 * @param computation - The function to compute the value of the store
 * @param dependencies - The stores it depends on
 * @example
 * ```ts
 * const foo = computed(() => bar.getValue() + baz.getValue(), [bar, baz]);
 * ```
 * @returns A store object
 */
export declare function computedStore<T>(computation: () => T, dependencies: (Store<any> | ReadonlyStore<any>)[]): ReadonlyStore<T>;
//# sourceMappingURL=computedStore.d.ts.map