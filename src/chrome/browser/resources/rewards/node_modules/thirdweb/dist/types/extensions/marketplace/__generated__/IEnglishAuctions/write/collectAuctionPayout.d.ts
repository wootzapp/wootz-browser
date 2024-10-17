import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "collectAuctionPayout" function.
 */
export type CollectAuctionPayoutParams = WithOverrides<{
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
}>;
export declare const FN_SELECTOR: "0xebf05a62";
/**
 * Checks if the `collectAuctionPayout` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `collectAuctionPayout` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCollectAuctionPayoutSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCollectAuctionPayoutSupported(["0x..."]);
 * ```
 */
export declare function isCollectAuctionPayoutSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "collectAuctionPayout" function.
 * @param options - The options for the collectAuctionPayout function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCollectAuctionPayoutParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCollectAuctionPayoutParams({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCollectAuctionPayoutParams(options: CollectAuctionPayoutParams): `0x${string}`;
/**
 * Encodes the "collectAuctionPayout" function into a Hex string with its parameters.
 * @param options - The options for the collectAuctionPayout function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCollectAuctionPayout } from "thirdweb/extensions/marketplace";
 * const result = encodeCollectAuctionPayout({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCollectAuctionPayout(options: CollectAuctionPayoutParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "collectAuctionPayout" function on the contract.
 * @param options - The options for the "collectAuctionPayout" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { collectAuctionPayout } from "thirdweb/extensions/marketplace";
 *
 * const transaction = collectAuctionPayout({
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
export declare function collectAuctionPayout(options: BaseTransactionOptions<CollectAuctionPayoutParams | {
    asyncParams: () => Promise<CollectAuctionPayoutParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=collectAuctionPayout.d.ts.map