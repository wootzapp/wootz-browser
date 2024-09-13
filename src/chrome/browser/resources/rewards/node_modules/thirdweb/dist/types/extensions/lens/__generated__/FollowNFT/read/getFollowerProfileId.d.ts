import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getFollowerProfileId" function.
 */
export type GetFollowerProfileIdParams = {
    followTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followTokenId";
    }>;
};
export declare const FN_SELECTOR: "0x886a65c3";
/**
 * Checks if the `getFollowerProfileId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getFollowerProfileId` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetFollowerProfileIdSupported } from "thirdweb/extensions/lens";
 * const supported = isGetFollowerProfileIdSupported(["0x..."]);
 * ```
 */
export declare function isGetFollowerProfileIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getFollowerProfileId" function.
 * @param options - The options for the getFollowerProfileId function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowerProfileIdParams } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowerProfileIdParams({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowerProfileIdParams(options: GetFollowerProfileIdParams): `0x${string}`;
/**
 * Encodes the "getFollowerProfileId" function into a Hex string with its parameters.
 * @param options - The options for the getFollowerProfileId function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetFollowerProfileId } from "thirdweb/extensions/lens";
 * const result = encodeGetFollowerProfileId({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetFollowerProfileId(options: GetFollowerProfileIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getFollowerProfileId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetFollowerProfileIdResult } from "thirdweb/extensions/lens";
 * const result = decodeGetFollowerProfileIdResultResult("...");
 * ```
 */
export declare function decodeGetFollowerProfileIdResult(result: Hex): bigint;
/**
 * Calls the "getFollowerProfileId" function on the contract.
 * @param options - The options for the getFollowerProfileId function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getFollowerProfileId } from "thirdweb/extensions/lens";
 *
 * const result = await getFollowerProfileId({
 *  contract,
 *  followTokenId: ...,
 * });
 *
 * ```
 */
export declare function getFollowerProfileId(options: BaseTransactionOptions<GetFollowerProfileIdParams>): Promise<bigint>;
//# sourceMappingURL=getFollowerProfileId.d.ts.map