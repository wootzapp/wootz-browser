import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type CancelAuctionParams = {
    auctionId: bigint;
};
/**
 * Cancels an auction by providing the necessary options.
 *
 * @param options - The options for canceling the auction.
 * @returns A transaction that can be sent to cancel the auction.
 * @throws An error when sending the transaction if the auction has an existing bid and cannot be canceled.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { cancelAuction } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = cancelAuction({
 *  contract,
 *  auctionId: 0n,
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function cancelAuction(options: BaseTransactionOptions<CancelAuctionParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
/**
 * Checks if the `cancelAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `cancelAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCancelAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCancelAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCancelAuctionSupported(availableSelectors: string[]): boolean;
//# sourceMappingURL=cancelAuction.d.ts.map