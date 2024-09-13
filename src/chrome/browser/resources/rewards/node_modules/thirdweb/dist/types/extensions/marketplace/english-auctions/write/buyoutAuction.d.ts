import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type BuyoutAuctionParams = {
    auctionId: bigint;
};
/**
 * Buys out an English auction.
 * @param options - The options for buying out the auction.
 * @returns A transaction that can be sent to buy out the auction.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { buyoutAuction } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = buyoutAuction({
 *  contract,
 *  auctionId: 0n
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function buyoutAuction(options: BaseTransactionOptions<BuyoutAuctionParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=buyoutAuction.d.ts.map