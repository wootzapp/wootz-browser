/**
 * Encodes a Uint8Array into a base58 string.
 * @param source - The Uint8Array to encode.
 * @returns The base58 encoded string.
 * @throws {TypeError} If the source is not a Uint8Array.
 * @throws {Error} If there is a non-zero carry during the encoding process.
 * @example
 * ```ts
 * import { base58Encode } from "thirdweb/utils;
 * const source = new Uint8Array([0, 1, 2, 3, 4, 5]);
 * const encoded = base58Encode(source);
 * console.log(encoded);
 * ```
 */
export declare function base58Encode(source: Uint8Array): string;
//# sourceMappingURL=encode.d.ts.map