import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Execute a Proposal
 * @extension VOTE
 * @returns a prepared transaction for the `execute` method
 *
 * @example
 * ```ts
 * import { executeProposal } from "thirdweb/extensions/vote";
 *
 * const transaction = executeProposal({ contract, proposalId });
 * const tx = await sendTransaction({ transaction, account });
 * ```
 */
export declare function executeProposal(options: BaseTransactionOptions<{
    proposalId: bigint;
}>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=executeProposal.d.ts.map