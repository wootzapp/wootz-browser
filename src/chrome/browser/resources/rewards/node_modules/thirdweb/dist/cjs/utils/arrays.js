"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.hasDuplicates = hasDuplicates;
/**
 * @internal
 */
function hasDuplicates(arr, fn) {
    if (arr.length === 0 || arr.length === 1) {
        return false;
    }
    if (!fn) {
        throw new Error("Comparison function required");
    }
    for (let i = 0; i < arr.length; i++) {
        for (let j = i + 1; j < arr.length; j++) {
            if (fn(arr[i], arr[j])) {
                return true;
            }
        }
    }
    return false;
}
//# sourceMappingURL=arrays.js.map