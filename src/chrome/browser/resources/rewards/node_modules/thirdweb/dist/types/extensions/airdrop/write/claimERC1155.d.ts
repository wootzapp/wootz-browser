import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * @extension AIRDROP
 */
export type ClaimERC1155Params = {
    tokenAddress: string;
    recipient: string;
};
/**
 * Claim airdrop of ERC1155 tokens for allowlisted addresses. (Pull based airdrop)
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { claimERC1155 } from "thirdweb/extensions/airdrop";
 * import { sendTransaction } from "thirdweb";
 *
 * const tokenAddress = "0x..." // Address of airdropped tokens to claim
 * const recipient = "0x..."  // Address of the allowlisted recipient
 *
 * const claimTransaction = claimERC1155({
 *    contract,
 *    tokenAddress,
 *    recipient
 * });
 *
 * await sendTransaction({ claimTransaction, account });
 *
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the transaction result.
 */
export declare function claimERC1155(options: BaseTransactionOptions<ClaimERC1155Params>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimERC1155.d.ts.map