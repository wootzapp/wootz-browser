/**
 * Timeout a promise with a given Error message if the promise does not resolve in given time
 * @internal
 */
export function timeoutPromise(promise, option) {
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