import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isFollowing" function.
 */
export type IsFollowingParams = {
    followerProfileId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followerProfileId";
    }>;
};
export declare const FN_SELECTOR: "0x4d71688d";
/**
 * Checks if the `isFollowing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isFollowing` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isIsFollowingSupported } from "thirdweb/extensions/lens";
 * const supported = isIsFollowingSupported(["0x..."]);
 * ```
 */
export declare function isIsFollowingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isFollowing" function.
 * @param options - The options for the isFollowing function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsFollowingParams } from "thirdweb/extensions/lens";
 * const result = encodeIsFollowingParams({
 *  followerProfileId: ...,
 * });
 * ```
 */
export declare function encodeIsFollowingParams(options: IsFollowingParams): `0x${string}`;
/**
 * Encodes the "isFollowing" function into a Hex string with its parameters.
 * @param options - The options for the isFollowing function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsFollowing } from "thirdweb/extensions/lens";
 * const result = encodeIsFollowing({
 *  followerProfileId: ...,
 * });
 * ```
 */
export declare function encodeIsFollowing(options: IsFollowingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isFollowing function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeIsFollowingResult } from "thirdweb/extensions/lens";
 * const result = decodeIsFollowingResultResult("...");
 * ```
 */
export declare function decodeIsFollowingResult(result: Hex): boolean;
/**
 * Calls the "isFollowing" function on the contract.
 * @param options - The options for the isFollowing function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { isFollowing } from "thirdweb/extensions/lens";
 *
 * const result = await isFollowing({
 *  contract,
 *  followerProfileId: ...,
 * });
 *
 * ```
 */
export declare function isFollowing(options: BaseTransactionOptions<IsFollowingParams>): Promise<boolean>;
//# sourceMappingURL=isFollowing.d.ts.map