import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "claimCondition" function.
 */
export type ClaimConditionParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
};
export declare const FN_SELECTOR: "0xe9703d25";
/**
 * Checks if the `claimCondition` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimCondition` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isClaimConditionSupported } from "thirdweb/extensions/erc1155";
 * const supported = isClaimConditionSupported(["0x..."]);
 * ```
 */
export declare function isClaimConditionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claimCondition" function.
 * @param options - The options for the claimCondition function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaimConditionParams } from "thirdweb/extensions/erc1155";
 * const result = encodeClaimConditionParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeClaimConditionParams(options: ClaimConditionParams): `0x${string}`;
/**
 * Encodes the "claimCondition" function into a Hex string with its parameters.
 * @param options - The options for the claimCondition function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaimCondition } from "thirdweb/extensions/erc1155";
 * const result = encodeClaimCondition({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeClaimCondition(options: ClaimConditionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the claimCondition function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeClaimConditionResult } from "thirdweb/extensions/erc1155";
 * const result = decodeClaimConditionResultResult("...");
 * ```
 */
export declare function decodeClaimConditionResult(result: Hex): readonly [bigint, bigint];
/**
 * Calls the "claimCondition" function on the contract.
 * @param options - The options for the claimCondition function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { claimCondition } from "thirdweb/extensions/erc1155";
 *
 * const result = await claimCondition({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function claimCondition(options: BaseTransactionOptions<ClaimConditionParams>): Promise<readonly [bigint, bigint]>;
//# sourceMappingURL=claimCondition.d.ts.map