export type Store<T> = {
    getValue(): T;
    setValue(newValue: T): void;
    subscribe(listener: () => void): () => void;
};
/**
 * Create a reactive value store
 * @param initialValue - The initial value to store
 * @example
 * ```ts
 * const store = createStore(0);
 * ```
 * @returns A store object
 * @internal
 */
export declare function createStore<T>(initialValue: T): Store<T>;
//# sourceMappingURL=store.d.ts.map