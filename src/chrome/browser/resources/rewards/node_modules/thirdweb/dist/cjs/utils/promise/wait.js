"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.wait = wait;
/**
 * Waits for the specified number of milliseconds.
 * @param ms - The number of milliseconds to wait.
 * @returns A promise that resolves after the specified time.
 * @internal
 */
function wait(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}
//# sourceMappingURL=wait.js.map