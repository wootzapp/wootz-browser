import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getFollowData" function.
 */
export type GetFollowDataParams = {
    followTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followTokenId";
    }>;
};
export declare const FN_SELECTOR: "0xd6cbec5d";
/**
 * Checks if the `getFollowData` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getFollowData` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetFollowDataSupported } from "thirdweb/extensions/lens";
 * const supported = isGetFollowDataSupported(["0x..."]);
 * ```
 */
export declare function isGetFollowDataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getFollowData" function.
 * @param options - The options for the getFollowData function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowDataParams } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowDataParams({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowDataParams(options: GetFollowDataParams): `0x${string}`;
/**
 * Encodes the "getFollowData" function into a Hex string with its parameters.
 * @param options - The options for the getFollowData function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowData } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowData({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowData(options: GetFollowDataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getFollowData function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetFollowDataResult } from "thirdweb/extensions/lens";
 * const result = decodeGetFollowDataResultResult("...");
 * ```
 */
export declare function decodeGetFollowDataResult(result: Hex): {
    followerProfileId: bigint;
    originalFollowTimestamp: number;
    followTimestamp: number;
    profileIdAllowedToRecover: bigint;
};
/**
 * Calls the "getFollowData" function on the contract.
 * @param options - The options for the getFollowData function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getFollowData } from "thirdweb/extensions/lens";
 *
 * const result = await getFollowData({
 *  contract,
 *  followTokenId: ...,
 * });
 *
 * ```
 */
export declare function getFollowData(options: BaseTransactionOptions<GetFollowDataParams>): Promise<{
    followerProfileId: bigint;
    originalFollowTimestamp: number;
    followTimestamp: number;
    profileIdAllowedToRecover: bigint;
}>;
//# sourceMappingURL=getFollowData.d.ts.map