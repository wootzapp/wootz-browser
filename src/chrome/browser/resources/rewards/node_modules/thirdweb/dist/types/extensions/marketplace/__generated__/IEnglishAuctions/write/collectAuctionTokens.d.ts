import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "collectAuctionTokens" function.
 */
export type CollectAuctionTokensParams = WithOverrides<{
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
}>;
export declare const FN_SELECTOR: "0x03a54fe0";
/**
 * Checks if the `collectAuctionTokens` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `collectAuctionTokens` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCollectAuctionTokensSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCollectAuctionTokensSupported(["0x..."]);
 * ```
 */
export declare function isCollectAuctionTokensSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "collectAuctionTokens" function.
 * @param options - The options for the collectAuctionTokens function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCollectAuctionTokensParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCollectAuctionTokensParams({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCollectAuctionTokensParams(options: CollectAuctionTokensParams): `0x${string}`;
/**
 * Encodes the "collectAuctionTokens" function into a Hex string with its parameters.
 * @param options - The options for the collectAuctionTokens function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCollectAuctionTokens } from "thirdweb/extensions/marketplace";
 * const result = encodeCollectAuctionTokens({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCollectAuctionTokens(options: CollectAuctionTokensParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "collectAuctionTokens" function on the contract.
 * @param options - The options for the "collectAuctionTokens" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { collectAuctionTokens } from "thirdweb/extensions/marketplace";
 *
 * const transaction = collectAuctionTokens({
 *  contract,
 *  auctionId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function collectAuctionTokens(options: BaseTransactionOptions<CollectAuctionTokensParams | {
    asyncParams: () => Promise<CollectAuctionTokensParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=collectAuctionTokens.d.ts.map