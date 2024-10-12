import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { Hex } from "../../../utils/encoding/hex.js";
import { type ProposalVoteInfo } from "./getProposalVoteCounts.js";
/**
 * @extension VOTE
 */
export type ProposalItem = {
    /**
     * ID of the proposal
     */
    proposalId: bigint;
    /**
     * The wallet address of the proposer
     */
    proposer: string;
    /**
     * Description of the proposal
     */
    description: string;
    /**
     * The block number at which the proposal is open for voting
     */
    startBlock: bigint;
    /**
     * The block number where the proposal concludes its voting phase
     */
    endBlock: bigint;
    /**
     * The current state of the proposal,  represented in number
     */
    state: number;
    /**
     * The current state of the proposal, represented in a user-friendly string
     * Example: "pending" | "active" | "canceled"
     */
    stateLabel: string | undefined;
    /**
     * The current vote info. See type [`ProposalVoteInfo`](https://portal.thirdweb.com/references/typescript/v5/ProposalItem) for more context
     */
    votes: ProposalVoteInfo;
    /**
     * The array of containing info about the set of actions that will be executed onchain,
     * should the proposal pass
     */
    executions: Array<{
        toAddress: string | undefined;
        nativeTokenValue: bigint | undefined;
        transactionData: Hex | undefined;
    }>;
};
/**
 * Get all proposals from a Vote contract with some extra info attached for each proposal (current state and votes)
 * @extension VOTE
 * @returns An array containing proposals data
 *
 * @example
 * ```ts
 * import { getAll } from "thirdweb/extension/getAll";
 *
 * const allProposals = await getAll({ contract });
 * ```
 */
export declare function getAll(options: BaseTransactionOptions): Promise<ProposalItem[]>;
//# sourceMappingURL=getAll.d.ts.map