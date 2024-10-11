import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type ExecuteSaleParams = {
    auctionId: bigint;
};
/**
 * Executes a sale for an English auction.
 *
 * @param options - The options for executing the sale.
 * @returns A transaction that can be sent to execute the sale.
 * @throws An error if the auction is still active and there is no winning bid.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { executeSale } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = executeSale({
 *  contract,
 *  auctionId: 0n,
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function executeSale(options: BaseTransactionOptions<ExecuteSaleParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=executeSale.d.ts.map