import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claimRewards" function.
 */
export type ClaimRewardsParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
}>;
export declare const FN_SELECTOR: "0x0962ef79";
/**
 * Checks if the `claimRewards` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimRewards` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isClaimRewardsSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isClaimRewardsSupported(["0x..."]);
 * ```
 */
export declare function isClaimRewardsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claimRewards" function.
 * @param options - The options for the claimRewards function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaimRewardsParams } from "thirdweb/extensions/erc1155";
 * const result = encodeClaimRewardsParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeClaimRewardsParams(options: ClaimRewardsParams): `0x${string}`;
/**
 * Encodes the "claimRewards" function into a Hex string with its parameters.
 * @param options - The options for the claimRewards function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaimRewards } from "thirdweb/extensions/erc1155";
 * const result = encodeClaimRewards({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeClaimRewards(options: ClaimRewardsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claimRewards" function on the contract.
 * @param options - The options for the "claimRewards" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimRewards } from "thirdweb/extensions/erc1155";
 *
 * const transaction = claimRewards({
 *  contract,
 *  tokenId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function claimRewards(options: BaseTransactionOptions<ClaimRewardsParams | {
    asyncParams: () => Promise<ClaimRewardsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimRewards.d.ts.map