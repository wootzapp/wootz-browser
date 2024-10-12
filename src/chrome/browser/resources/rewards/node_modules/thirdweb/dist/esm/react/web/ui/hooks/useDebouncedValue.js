import { useEffect, useState } from "react";
/**
 * @internal
 */
export function useDebouncedValue(value, delay) {
    const [debouncedValue, setDebouncedValue] = useState(value);
    useEffect(() => {
        let ignore = false;
        const id = setTimeout(() => {
            if (ignore) {
                return;
            }
            setDebouncedValue(value);
        }, delay);
        return () => {
            ignore = true;
            clearTimeout(id);
        };
    }, [value, delay]);
    return debouncedValue;
}
//# sourceMappingURL=useDebouncedValue.js.map