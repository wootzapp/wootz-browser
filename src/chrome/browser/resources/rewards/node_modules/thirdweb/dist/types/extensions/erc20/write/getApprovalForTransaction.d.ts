import type { PreparedTransaction } from "../../../transaction/prepare-transaction.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
export type GetApprovalForTransactionParams = {
    /**
     * The transaction that involves the ERC20 token
     */
    transaction: PreparedTransaction;
    /**
     * The caller's account
     */
    account: Account;
};
/**
 * When dealing with transactions that involve ERC20 tokens (Airdropping ERC20, buy NFTs with ERC20, etc.)
 * you often have to do a pre-check to see if the targeted contract has the sufficient allowance to "take" the ERC20 tokens from the caller's wallet.
 *
 * This extension is a handy method that checks for the allowance and requests to approve for more if current allowance is insufficient
 *
 * @param options GetApprovalForTransactionParams
 * @returns a PreparedTransaction
 *
 * @example
 * ```ts
 * import { getApprovalForTransaction } from "thirdweb/extensions/erc20";
 * import { sendAndConfirmTransaction } from "thirdweb";
 *
 * async function buyNFT() {
 *   const buyTransaction = ... // could be a marketplacev3's buyFromListing
 *
 *   // Check if you need to approve spending for the involved ERC20 contract
 *   const approveTx = await getApprovalForTransaction({
 *     transaction: buyTransaction,
 *     account, // the connected account
 *   });
 *   if (approveTx) {
 *     await sendAndConfirmTransaction({
 *       transaction: approveTx,
 *       account,
 *     })
 *   }
 *   // Once approved, you can finally perform the buy transaction
 *   await sendAndConfirmTransaction({
 *     transaction: buyTransaction,
 *     account,
 *   });
 * }
 * ```
 *
 * @transaction
 */
export declare function getApprovalForTransaction(options: GetApprovalForTransactionParams): Promise<PreparedTransaction | null>;
//# sourceMappingURL=getApprovalForTransaction.d.ts.map