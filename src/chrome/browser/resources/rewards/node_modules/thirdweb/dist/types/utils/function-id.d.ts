type AnyFunction = (...args: any[]) => any;
/**
 * Retrieves the unique identifier for a given function.
 * If the function has been previously cached, the cached identifier is returned.
 * Otherwise, a new identifier is generated using the function's string representation.
 * @param fn - The function for which to retrieve the identifier.
 * @returns The unique identifier for the function.
 * @internal
 */
export declare function getFunctionId(fn: AnyFunction): string;
export {};
//# sourceMappingURL=function-id.d.ts.map