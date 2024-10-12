/**
 * Check if the given value is an instance of `Uint8Array`.
 *
 * Replacement for [`Buffer.isBuffer()`](https://nodejs.org/api/buffer.html#static-method-bufferisbufferobj).
 * @example
 * ```ts
 * import {isUint8Array} from 'uint8array-extras';
 *
 * console.log(isUint8Array(new Uint8Array()));
 * //=> true
 *
 * console.log(isUint8Array(Buffer.from('x')));
 * //=> true
 *
 * console.log(isUint8Array(new ArrayBuffer(10)));
 * //=> false
 * ```
 */
export declare function isUint8Array(value: unknown): value is Uint8Array;
/**
 * Check if two arrays are identical by verifying that they contain the same bytes in the same sequence.
 *
 * Replacement for [`Buffer#equals()`](https://nodejs.org/api/buffer.html#bufequalsotherbuffer).
 * @example
 * ```ts
 * import {areUint8ArraysEqual} from 'uint8array-extras';
 *
 * const a = new Uint8Array([1, 2, 3]);
 * const b = new Uint8Array([1, 2, 3]);
 * const c = new Uint8Array([4, 5, 6]);
 *
 * console.log(areUint8ArraysEqual(a, b));
 * //=> true
 *
 * console.log(areUint8ArraysEqual(a, c));
 * //=> false
 * ```
 */
export declare function areUint8ArraysEqual(a: Uint8Array, b: Uint8Array): boolean;
export declare function base64UrlToBase64(base64url: string): string;
/**
 * Convert a Base64-encoded or [Base64URL](https://base64.guru/standards/base64url)-encoded string to a `Uint8Array`.
 *
 * Replacement for [`Buffer.from('SGVsbG8=', 'base64')`](https://nodejs.org/api/buffer.html#static-method-bufferfromstring-encoding).
 * @example
 * ```ts
 * import {base64ToUint8Array} from 'uint8array-extras';
 *
 * console.log(base64ToUint8Array('SGVsbG8='));
 * //=> Uint8Array [72, 101, 108, 108, 111]
 * ```
 */
export declare function base64ToUint8Array(base64String: string): Uint8Array;
/**
 * Decode a Base64-encoded or [Base64URL](https://base64.guru/standards/base64url)-encoded string to a string.
 *
 * Replacement for `Buffer.from('SGVsbG8=', 'base64').toString()` and [`atob()`](https://developer.mozilla.org/en-US/docs/Web/API/atob).
 * @example
 * ```ts
 * import {base64ToString} from 'uint8array-extras';
 *
 * console.log(base64ToString('SGVsbG8='));
 * //=> 'Hello'
 * ```
 */
export declare function base64ToString(base64String: string): string;
export declare function uint8ArrayToBase64(array: Uint8Array, { urlSafe }?: {
    urlSafe?: boolean | undefined;
}): string;
export declare function concatUint8Arrays(arrays: Uint8Array[], totalLength?: number): Uint8Array;
export declare function compareUint8Arrays(a: Uint8Array, b: Uint8Array): number;
//# sourceMappingURL=uint8-array.d.ts.map