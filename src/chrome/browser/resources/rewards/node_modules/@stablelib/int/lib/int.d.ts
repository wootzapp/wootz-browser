/** 32-bit integer multiplication.  */
export declare const mul: (a: number, b: number) => number;
/** 32-bit integer addition.  */
export declare function add(a: number, b: number): number;
/**  32-bit integer subtraction.  */
export declare function sub(a: number, b: number): number;
/** 32-bit integer left rotation */
export declare function rotl(x: number, n: number): number;
/** 32-bit integer left rotation */
export declare function rotr(x: number, n: number): number;
/**
 * Returns true if the argument is an integer number.
 *
 * In ES2015, Number.isInteger.
 */
export declare const isInteger: (n: number) => boolean;
/**
 *  Math.pow(2, 53) - 1
 *
 *  In ES2015 Number.MAX_SAFE_INTEGER.
 */
export declare const MAX_SAFE_INTEGER = 9007199254740991;
/**
 * Returns true if the argument is a safe integer number
 * (-MIN_SAFE_INTEGER < number <= MAX_SAFE_INTEGER)
 *
 * In ES2015, Number.isSafeInteger.
 */
export declare const isSafeInteger: (n: number) => boolean;
