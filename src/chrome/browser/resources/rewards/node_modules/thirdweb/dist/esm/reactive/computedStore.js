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
export function computedStore(
// pass the values of the dependencies to the computation function
computation, 
// biome-ignore lint/suspicious/noExplicitAny: library function that accepts any store type
dependencies) {
    const listeners = new Set();
    let value = computation();
    const notify = () => {
        for (const listener of listeners) {
            listener();
        }
    };
    const setValue = (newValue) => {
        value = newValue;
        notify();
    };
    // when any of the dependencies change, recompute the value and set it
    for (const store of dependencies) {
        store.subscribe(() => {
            setValue(computation());
        });
    }
    return {
        getValue() {
            return value;
        },
        subscribe(listener) {
            listeners.add(listener);
            return () => {
                listeners.delete(listener);
            };
        },
    };
}
//# sourceMappingURL=computedStore.js.map