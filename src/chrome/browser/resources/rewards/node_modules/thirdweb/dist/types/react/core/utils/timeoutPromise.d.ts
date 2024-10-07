/**
 * Timeout a promise with a given Error message if the promise does not resolve in given time
 * @internal
 */
export declare function timeoutPromise<T>(promise: Promise<T>, option: {
    ms: number;
    message: string;
}): Promise<T>;
//# sourceMappingURL=timeoutPromise.d.ts.map