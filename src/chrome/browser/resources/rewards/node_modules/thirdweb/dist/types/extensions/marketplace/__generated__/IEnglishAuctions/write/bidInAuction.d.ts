import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "bidInAuction" function.
 */
export type BidInAuctionParams = WithOverrides<{
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
    bidAmount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_bidAmount";
    }>;
}>;
export declare const FN_SELECTOR: "0x0858e5ad";
/**
 * Checks if the `bidInAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `bidInAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isBidInAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isBidInAuctionSupported(["0x..."]);
 * ```
 */
export declare function isBidInAuctionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "bidInAuction" function.
 * @param options - The options for the bidInAuction function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeBidInAuctionParams } from "thirdweb/extensions/marketplace";
 * const result = encodeBidInAuctionParams({
 *  auctionId: ...,
 *  bidAmount: ...,
 * });
 * ```
 */
export declare function encodeBidInAuctionParams(options: BidInAuctionParams): `0x${string}`;
/**
 * Encodes the "bidInAuction" function into a Hex string with its parameters.
 * @param options - The options for the bidInAuction function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeBidInAuction } from "thirdweb/extensions/marketplace";
 * const result = encodeBidInAuction({
 *  auctionId: ...,
 *  bidAmount: ...,
 * });
 * ```
 */
export declare function encodeBidInAuction(options: BidInAuctionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "bidInAuction" function on the contract.
 * @param options - The options for the "bidInAuction" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { bidInAuction } from "thirdweb/extensions/marketplace";
 *
 * const transaction = bidInAuction({
 *  contract,
 *  auctionId: ...,
 *  bidAmount: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function bidInAuction(options: BaseTransactionOptions<BidInAuctionParams | {
    asyncParams: () => Promise<BidInAuctionParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=bidInAuction.d.ts.map