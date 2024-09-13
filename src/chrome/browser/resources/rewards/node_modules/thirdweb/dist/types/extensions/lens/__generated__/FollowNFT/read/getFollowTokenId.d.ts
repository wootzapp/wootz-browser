import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getFollowTokenId" function.
 */
export type GetFollowTokenIdParams = {
    followerProfileId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followerProfileId";
    }>;
};
export declare const FN_SELECTOR: "0x11c763d6";
/**
 * Checks if the `getFollowTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getFollowTokenId` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetFollowTokenIdSupported } from "thirdweb/extensions/lens";
 * const supported = isGetFollowTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isGetFollowTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getFollowTokenId" function.
 * @param options - The options for the getFollowTokenId function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowTokenIdParams } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowTokenIdParams({
 *  followerProfileId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowTokenIdParams(options: GetFollowTokenIdParams): `0x${string}`;
/**
 * Encodes the "getFollowTokenId" function into a Hex string with its parameters.
 * @param options - The options for the getFollowTokenId function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowTokenId } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowTokenId({
 *  followerProfileId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowTokenId(options: GetFollowTokenIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getFollowTokenId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetFollowTokenIdResult } from "thirdweb/extensions/lens";
 * const result = decodeGetFollowTokenIdResultResult("...");
 * ```
 */
export declare function decodeGetFollowTokenIdResult(result: Hex): bigint;
/**
 * Calls the "getFollowTokenId" function on the contract.
 * @param options - The options for the getFollowTokenId function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getFollowTokenId } from "thirdweb/extensions/lens";
 *
 * const result = await getFollowTokenId({
 *  contract,
 *  followerProfileId: ...,
 * });
 *
 * ```
 */
export declare function getFollowTokenId(options: BaseTransactionOptions<GetFollowTokenIdParams>): Promise<bigint>;
//# sourceMappingURL=getFollowTokenId.d.ts.map