import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type BidInAuctionParams = {
    auctionId: bigint;
} & ({
    bidAmountWei: bigint;
} | {
    bidAmount: string;
});
/**
 * Places a bid in an English auction.
 * @param options - The options for placing the bid.
 * @returns A transaction that can be sent to place the bid.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { bidInAuction } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = bidInAuction({
 *  contract,
 *  auctionId: 0n,
 *  bidAmount: "100",
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function bidInAuction(options: BaseTransactionOptions<BidInAuctionParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
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
//# sourceMappingURL=bidInAuction.d.ts.map