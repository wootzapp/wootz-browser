import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "keysOf" function.
 */
export type KeysOfParams = {
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
    }>;
    state: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "state";
    }>;
};
export declare const FN_SELECTOR: "0x1f64222f";
/**
 * Checks if the `keysOf` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `keysOf` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isKeysOfSupported } from "thirdweb/extensions/farcaster";
 * const supported = isKeysOfSupported(["0x..."]);
 * ```
 */
export declare function isKeysOfSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "keysOf" function.
 * @param options - The options for the keysOf function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeKeysOfParams } from "thirdweb/extensions/farcaster";
 * const result = encodeKeysOfParams({
 *  fid: ...,
 *  state: ...,
 * });
 * ```
 */
export declare function encodeKeysOfParams(options: KeysOfParams): `0x${string}`;
/**
 * Encodes the "keysOf" function into a Hex string with its parameters.
 * @param options - The options for the keysOf function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeKeysOf } from "thirdweb/extensions/farcaster";
 * const result = encodeKeysOf({
 *  fid: ...,
 *  state: ...,
 * });
 * ```
 */
export declare function encodeKeysOf(options: KeysOfParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the keysOf function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeKeysOfResult } from "thirdweb/extensions/farcaster";
 * const result = decodeKeysOfResultResult("...");
 * ```
 */
export declare function decodeKeysOfResult(result: Hex): readonly `0x${string}`[];
/**
 * Calls the "keysOf" function on the contract.
 * @param options - The options for the keysOf function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { keysOf } from "thirdweb/extensions/farcaster";
 *
 * const result = await keysOf({
 *  contract,
 *  fid: ...,
 *  state: ...,
 * });
 *
 * ```
 */
export declare function keysOf(options: BaseTransactionOptions<KeysOfParams>): Promise<readonly `0x${string}`[]>;
//# sourceMappingURL=keysOf.d.ts.map