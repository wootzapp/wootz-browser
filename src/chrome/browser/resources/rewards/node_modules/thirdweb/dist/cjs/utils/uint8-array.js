"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isUint8Array = isUint8Array;
exports.areUint8ArraysEqual = areUint8ArraysEqual;
exports.base64UrlToBase64 = base64UrlToBase64;
exports.base64ToUint8Array = base64ToUint8Array;
exports.base64ToString = base64ToString;
exports.uint8ArrayToBase64 = uint8ArrayToBase64;
exports.concatUint8Arrays = concatUint8Arrays;
exports.compareUint8Arrays = compareUint8Arrays;
const text_decoder_js_1 = require("./text-decoder.js");
const uint8ArrayStringified = "[object Uint8Array]";
/**
 * Throw a `TypeError` if the given value is not an instance of `Uint8Array`.
 * @example
 * ```ts
 * import {assertUint8Array} from 'uint8array-extras';
 *
 * try {
 * assertUint8Array(new ArrayBuffer(10)); // Throws a TypeError
 * } catch (error) {
 * console.error(error.message);
 * }
 * ```
 */
function assertUint8Array(value) {
    if (!isUint8Array(value)) {
        throw new TypeError(`Expected \`Uint8Array\`, got \`${typeof value}\``);
    }
}
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
function isUint8Array(value) {
    if (!value) {
        return false;
    }
    if (value.constructor === Uint8Array) {
        return true;
    }
    return Object.prototype.toString.call(value) === uint8ArrayStringified;
}
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
function areUint8ArraysEqual(a, b) {
    assertUint8Array(a);
    assertUint8Array(b);
    if (a === b) {
        return true;
    }
    if (a.length !== b.length) {
        return false;
    }
    for (let index = 0; index < a.length; index++) {
        if (a[index] !== b[index]) {
            return false;
        }
    }
    return true;
}
/**
 * Convert a `Uint8Array` (containing a UTF-8 string) to a string.
 *
 * Replacement for [`Buffer#toString()`](https://nodejs.org/api/buffer.html#buftostringencoding-start-end).
 * @example
 * ```ts
 * import {uint8ArrayToString} from 'uint8array-extras';
 *
 * const byteArray = new Uint8Array([72, 101, 108, 108, 111]);
 *
 * console.log(uint8ArrayToString(byteArray));
 * //=> 'Hello'
 * ```
 */
function uint8ArrayToString(array) {
    assertUint8Array(array);
    return (0, text_decoder_js_1.cachedTextDecoder)().decode(array);
}
function assertString(value) {
    if (typeof value !== "string") {
        throw new TypeError(`Expected \`string\`, got \`${typeof value}\``);
    }
}
function base64UrlToBase64(base64url) {
    // Replace Base64URL characters with Base64 characters
    let base64 = base64url.replace(/-/g, "+").replace(/_/g, "/");
    // Add padding if necessary
    const padding = base64.length % 4;
    if (padding !== 0) {
        base64 += "=".repeat(4 - padding);
    }
    return base64;
}
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
function base64ToUint8Array(base64String) {
    assertString(base64String);
    return Uint8Array.from(globalThis.atob(base64UrlToBase64(base64String)), 
    // biome-ignore lint/style/noNonNullAssertion: we know that the code points exist
    (x) => x.codePointAt(0));
}
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
function base64ToString(base64String) {
    assertString(base64String);
    return uint8ArrayToString(base64ToUint8Array(base64String));
}
function base64ToBase64Url(base64) {
    return base64.replaceAll("+", "-").replaceAll("/", "_").replace(/=+$/, "");
}
// Reference: https://phuoc.ng/collection/this-vs-that/concat-vs-push/
const MAX_BLOCK_SIZE = 65_535;
function uint8ArrayToBase64(array, { urlSafe = false } = {}) {
    assertUint8Array(array);
    let base64;
    if (array.length < MAX_BLOCK_SIZE) {
        // Required as `btoa` and `atob` don't properly support Unicode: https://developer.mozilla.org/en-US/docs/Glossary/Base64#the_unicode_problem
        // @ts-expect-error - TS doesn't know about `String#fromCodePoint`
        base64 = globalThis.btoa(String.fromCodePoint.apply(this, array));
    }
    else {
        base64 = "";
        for (const value of array) {
            base64 += String.fromCodePoint(value);
        }
        base64 = globalThis.btoa(base64);
    }
    return urlSafe ? base64ToBase64Url(base64) : base64;
}
function concatUint8Arrays(arrays, totalLength) {
    if (arrays.length === 0) {
        return new Uint8Array(0);
    }
    const calculatedTotalLength = totalLength ??
        arrays.reduce((accumulator, currentValue) => accumulator + currentValue.length, 0);
    const returnValue = new Uint8Array(calculatedTotalLength);
    let offset = 0;
    for (const array of arrays) {
        assertUint8Array(array);
        returnValue.set(array, offset);
        offset += array.length;
    }
    return returnValue;
}
function compareUint8Arrays(a, b) {
    assertUint8Array(a);
    assertUint8Array(b);
    const length = Math.min(a.length, b.length);
    for (let index = 0; index < length; index++) {
        // biome-ignore lint/style/noNonNullAssertion: we check the length above so the index is always in bounds
        const diff = a[index] - b[index];
        if (diff !== 0) {
            return Math.sign(diff);
        }
    }
    // At this point, all the compared elements are equal.
    // The shorter array should come first if the arrays are of different lengths.
    return Math.sign(a.length - b.length);
}
//# sourceMappingURL=uint8-array.js.map