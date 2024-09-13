import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "canClaimRewards" function.
 */
export type CanClaimRewardsParams = {
    opener: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_opener";
    }>;
};
export declare const FN_SELECTOR: "0xa9b47a66";
/**
 * Checks if the `canClaimRewards` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `canClaimRewards` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isCanClaimRewardsSupported } from "thirdweb/extensions/erc1155";
 * const supported = isCanClaimRewardsSupported(["0x..."]);
 * ```
 */
export declare function isCanClaimRewardsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "canClaimRewards" function.
 * @param options - The options for the canClaimRewards function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeCanClaimRewardsParams } from "thirdweb/extensions/erc1155";
 * const result = encodeCanClaimRewardsParams({
 *  opener: ...,
 * });
 * ```
 */
export declare function encodeCanClaimRewardsParams(options: CanClaimRewardsParams): `0x${string}`;
/**
 * Encodes the "canClaimRewards" function into a Hex string with its parameters.
 * @param options - The options for the canClaimRewards function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeCanClaimRewards } from "thirdweb/extensions/erc1155";
 * const result = encodeCanClaimRewards({
 *  opener: ...,
 * });
 * ```
 */
export declare function encodeCanClaimRewards(options: CanClaimRewardsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the canClaimRewards function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeCanClaimRewardsResult } from "thirdweb/extensions/erc1155";
 * const result = decodeCanClaimRewardsResultResult("...");
 * ```
 */
export declare function decodeCanClaimRewardsResult(result: Hex): boolean;
/**
 * Calls the "canClaimRewards" function on the contract.
 * @param options - The options for the canClaimRewards function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { canClaimRewards } from "thirdweb/extensions/erc1155";
 *
 * const result = await canClaimRewards({
 *  contract,
 *  opener: ...,
 * });
 *
 * ```
 */
export declare function canClaimRewards(options: BaseTransactionOptions<CanClaimRewardsParams>): Promise<boolean>;
//# sourceMappingURL=canClaimRewards.d.ts.map