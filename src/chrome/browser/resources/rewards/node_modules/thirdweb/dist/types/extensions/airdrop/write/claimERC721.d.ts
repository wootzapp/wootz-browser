import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * @extension AIRDROP
 */
export type ClaimERC721Params = {
    tokenAddress: string;
    recipient: string;
};
/**
 * Claim airdrop of ERC721 tokens for allowlisted addresses. (Pull based airdrop)
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { claimERC721 } from "thirdweb/extensions/airdrop";
 * import { sendTransaction } from "thirdweb";
 *
 * const tokenAddress = "0x..." // Address of airdropped tokens to claim
 * const recipient = "0x..."  // Address of the allowlisted recipient
 *
 * const claimTransaction = claimERC721({
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
export declare function claimERC721(options: BaseTransactionOptions<ClaimERC721Params>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimERC721.d.ts.map