/**
 * Attempts to execute a function that returns a promise and retries if the function throws an error.
 *
 * @param {Function} fn - A function that returns a promise to be executed.
 * @param {Object} options - Configuration options for the retry behavior.
 * @param {number} [options.retries=1] - The number of times to retry the function before failing.
 * @param {number} [options.delay=0] - The delay in milliseconds between retries.
 * @returns {Promise<void>} The result of the function execution if successful.
 */
export declare function retry<T>(fn: () => Promise<T>, options: {
    retries?: number;
    delay?: number;
}): Promise<T>;
//# sourceMappingURL=retry.d.ts.map