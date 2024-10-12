import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getOriginalFollowTimestamp" function.
 */
export type GetOriginalFollowTimestampParams = {
    followTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followTokenId";
    }>;
};
export declare const FN_SELECTOR: "0xd1b34934";
/**
 * Checks if the `getOriginalFollowTimestamp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getOriginalFollowTimestamp` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetOriginalFollowTimestampSupported } from "thirdweb/extensions/lens";
 * const supported = isGetOriginalFollowTimestampSupported(["0x..."]);
 * ```
 */
export declare function isGetOriginalFollowTimestampSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getOriginalFollowTimestamp" function.
 * @param options - The options for the getOriginalFollowTimestamp function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetOriginalFollowTimestampParams } from "thirdweb/extensions/lens";
 * const result = encodeGetOriginalFollowTimestampParams({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetOriginalFollowTimestampParams(options: GetOriginalFollowTimestampParams): `0x${string}`;
/**
 * Encodes the "getOriginalFollowTimestamp" function into a Hex string with its parameters.
 * @param options - The options for the getOriginalFollowTimestamp function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetOriginalFollowTimestamp } from "thirdweb/extensions/lens";
 * const result = encodeGetOriginalFollowTimestamp({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetOriginalFollowTimestamp(options: GetOriginalFollowTimestampParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getOriginalFollowTimestamp function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetOriginalFollowTimestampResult } from "thirdweb/extensions/lens";
 * const result = decodeGetOriginalFollowTimestampResultResult("...");
 * ```
 */
export declare function decodeGetOriginalFollowTimestampResult(result: Hex): bigint;
/**
 * Calls the "getOriginalFollowTimestamp" function on the contract.
 * @param options - The options for the getOriginalFollowTimestamp function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getOriginalFollowTimestamp } from "thirdweb/extensions/lens";
 *
 * const result = await getOriginalFollowTimestamp({
 *  contract,
 *  followTokenId: ...,
 * });
 *
 * ```
 */
export declare function getOriginalFollowTimestamp(options: BaseTransactionOptions<GetOriginalFollowTimestampParams>): Promise<bigint>;
//# sourceMappingURL=getOriginalFollowTimestamp.d.ts.map