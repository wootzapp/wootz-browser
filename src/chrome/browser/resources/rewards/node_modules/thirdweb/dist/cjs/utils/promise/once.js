"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.once = once;
function once(fn) {
    let result;
    return () => {
        if (!result) {
            result = fn();
        }
        return result;
    };
}
//# sourceMappingURL=once.js.map