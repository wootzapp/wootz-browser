"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useDebouncedValue = useDebouncedValue;
const react_1 = require("react");
/**
 * @internal
 */
function useDebouncedValue(value, delay) {
    const [debouncedValue, setDebouncedValue] = (0, react_1.useState)(value);
    (0, react_1.useEffect)(() => {
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