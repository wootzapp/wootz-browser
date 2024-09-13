import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "keyAt" function.
 */
export type KeyAtParams = {
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
    }>;
    state: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "state";
    }>;
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "index";
    }>;
};
export declare const FN_SELECTOR: "0x0ea9442c";
/**
 * Checks if the `keyAt` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `keyAt` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isKeyAtSupported } from "thirdweb/extensions/farcaster";
 * const supported = isKeyAtSupported(["0x..."]);
 * ```
 */
export declare function isKeyAtSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "keyAt" function.
 * @param options - The options for the keyAt function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeKeyAtParams } from "thirdweb/extensions/farcaster";
 * const result = encodeKeyAtParams({
 *  fid: ...,
 *  state: ...,
 *  index: ...,
 * });
 * ```
 */
export declare function encodeKeyAtParams(options: KeyAtParams): `0x${string}`;
/**
 * Encodes the "keyAt" function into a Hex string with its parameters.
 * @param options - The options for the keyAt function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeKeyAt } from "thirdweb/extensions/farcaster";
 * const result = encodeKeyAt({
 *  fid: ...,
 *  state: ...,
 *  index: ...,
 * });
 * ```
 */
export declare function encodeKeyAt(options: KeyAtParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the keyAt function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeKeyAtResult } from "thirdweb/extensions/farcaster";
 * const result = decodeKeyAtResultResult("...");
 * ```
 */
export declare function decodeKeyAtResult(result: Hex): `0x${string}`;
/**
 * Calls the "keyAt" function on the contract.
 * @param options - The options for the keyAt function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { keyAt } from "thirdweb/extensions/farcaster";
 *
 * const result = await keyAt({
 *  contract,
 *  fid: ...,
 *  state: ...,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function keyAt(options: BaseTransactionOptions<KeyAtParams>): Promise<`0x${string}`>;
//# sourceMappingURL=keyAt.d.ts.map