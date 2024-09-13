import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getFollowTimestamp" function.
 */
export type GetFollowTimestampParams = {
    followTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followTokenId";
    }>;
};
export declare const FN_SELECTOR: "0x3543a277";
/**
 * Checks if the `getFollowTimestamp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getFollowTimestamp` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetFollowTimestampSupported } from "thirdweb/extensions/lens";
 * const supported = isGetFollowTimestampSupported(["0x..."]);
 * ```
 */
export declare function isGetFollowTimestampSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getFollowTimestamp" function.
 * @param options - The options for the getFollowTimestamp function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowTimestampParams } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowTimestampParams({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowTimestampParams(options: GetFollowTimestampParams): `0x${string}`;
/**
 * Encodes the "getFollowTimestamp" function into a Hex string with its parameters.
 * @param options - The options for the getFollowTimestamp function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowTimestamp } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowTimestamp({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowTimestamp(options: GetFollowTimestampParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getFollowTimestamp function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetFollowTimestampResult } from "thirdweb/extensions/lens";
 * const result = decodeGetFollowTimestampResultResult("...");
 * ```
 */
export declare function decodeGetFollowTimestampResult(result: Hex): bigint;
/**
 * Calls the "getFollowTimestamp" function on the contract.
 * @param options - The options for the getFollowTimestamp function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getFollowTimestamp } from "thirdweb/extensions/lens";
 *
 * const result = await getFollowTimestamp({
 *  contract,
 *  followTokenId: ...,
 * });
 *
 * ```
 */
export declare function getFollowTimestamp(options: BaseTransactionOptions<GetFollowTimestampParams>): Promise<bigint>;
//# sourceMappingURL=getFollowTimestamp.d.ts.map