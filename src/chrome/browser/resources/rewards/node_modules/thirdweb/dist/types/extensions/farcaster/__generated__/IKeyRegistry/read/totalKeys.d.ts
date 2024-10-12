import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "totalKeys" function.
 */
export type TotalKeysParams = {
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
    }>;
    state: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "state";
    }>;
};
export declare const FN_SELECTOR: "0x6840b75e";
/**
 * Checks if the `totalKeys` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalKeys` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTotalKeysSupported } from "thirdweb/extensions/farcaster";
 * const supported = isTotalKeysSupported(["0x..."]);
 * ```
 */
export declare function isTotalKeysSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "totalKeys" function.
 * @param options - The options for the totalKeys function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTotalKeysParams } from "thirdweb/extensions/farcaster";
 * const result = encodeTotalKeysParams({
 *  fid: ...,
 *  state: ...,
 * });
 * ```
 */
export declare function encodeTotalKeysParams(options: TotalKeysParams): `0x${string}`;
/**
 * Encodes the "totalKeys" function into a Hex string with its parameters.
 * @param options - The options for the totalKeys function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTotalKeys } from "thirdweb/extensions/farcaster";
 * const result = encodeTotalKeys({
 *  fid: ...,
 *  state: ...,
 * });
 * ```
 */
export declare function encodeTotalKeys(options: TotalKeysParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the totalKeys function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeTotalKeysResult } from "thirdweb/extensions/farcaster";
 * const result = decodeTotalKeysResultResult("...");
 * ```
 */
export declare function decodeTotalKeysResult(result: Hex): bigint;
/**
 * Calls the "totalKeys" function on the contract.
 * @param options - The options for the totalKeys function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { totalKeys } from "thirdweb/extensions/farcaster";
 *
 * const result = await totalKeys({
 *  contract,
 *  fid: ...,
 *  state: ...,
 * });
 *
 * ```
 */
export declare function totalKeys(options: BaseTransactionOptions<TotalKeysParams>): Promise<bigint>;
//# sourceMappingURL=totalKeys.d.ts.map