"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.timeoutPromise = timeoutPromise;
/**
 * Timeout a promise with a given Error message if the promise does not resolve in given time
 * @internal
 */
function timeoutPromise(promise, option) {
    return new Promise((resolve, reject) => {
        const timeoutId = setTimeout(() => {
            reject(new Error(option.message));
        }, option.ms);
        promise.then((res) => {
            clearTimeout(timeoutId);
            resolve(res);
        }, (err) => {
            clearTimeout(timeoutId);
            reject(err);
        });
    });
}
//# sourceMappingURL=timeoutPromise.js.map