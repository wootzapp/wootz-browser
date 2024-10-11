import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getActiveClaimConditionId" function.
 */
export type GetActiveClaimConditionIdParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x5ab063e8";
/**
 * Checks if the `getActiveClaimConditionId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getActiveClaimConditionId` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isGetActiveClaimConditionIdSupported } from "thirdweb/extensions/erc1155";
 * const supported = isGetActiveClaimConditionIdSupported(["0x..."]);
 * ```
 */
export declare function isGetActiveClaimConditionIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getActiveClaimConditionId" function.
 * @param options - The options for the getActiveClaimConditionId function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetActiveClaimConditionIdParams } from "thirdweb/extensions/erc1155";
 * const result = encodeGetActiveClaimConditionIdParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetActiveClaimConditionIdParams(options: GetActiveClaimConditionIdParams): `0x${string}`;
/**
 * Encodes the "getActiveClaimConditionId" function into a Hex string with its parameters.
 * @param options - The options for the getActiveClaimConditionId function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetActiveClaimConditionId } from "thirdweb/extensions/erc1155";
 * const result = encodeGetActiveClaimConditionId({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetActiveClaimConditionId(options: GetActiveClaimConditionIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getActiveClaimConditionId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeGetActiveClaimConditionIdResult } from "thirdweb/extensions/erc1155";
 * const result = decodeGetActiveClaimConditionIdResultResult("...");
 * ```
 */
export declare function decodeGetActiveClaimConditionIdResult(result: Hex): bigint;
/**
 * Calls the "getActiveClaimConditionId" function on the contract.
 * @param options - The options for the getActiveClaimConditionId function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getActiveClaimConditionId } from "thirdweb/extensions/erc1155";
 *
 * const result = await getActiveClaimConditionId({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function getActiveClaimConditionId(options: BaseTransactionOptions<GetActiveClaimConditionIdParams>): Promise<bigint>;
//# sourceMappingURL=getActiveClaimConditionId.d.ts.map