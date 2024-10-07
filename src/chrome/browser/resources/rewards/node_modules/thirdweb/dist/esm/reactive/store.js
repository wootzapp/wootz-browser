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
export function createStore(initialValue) {
    const listeners = new Set();
    let value = initialValue;
    const notify = () => {
        for (const listener of listeners) {
            listener();
        }
    };
    return {
        getValue() {
            return value;
        },
        setValue(newValue) {
            if (newValue === value) {
                return;
            }
            value = newValue;
            notify();
        },
        subscribe(listener) {
            listeners.add(listener);
            return () => {
                listeners.delete(listener);
            };
        },
    };
}
//# sourceMappingURL=store.js.map