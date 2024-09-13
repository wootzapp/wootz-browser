import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Simulate the `execute` method of the Vote contract, to check if you can execute a proposal
 * @extension VOTE
 * @returns boolean - `true` if the proposal is executable, else `false`
 *
 * @example
 * ```ts
 * import { canExecute } from "thirdweb/extensions/vote";
 *
 * const executable = await canExecute({ contract, proposalId });
 * ```
 */
export declare function canExecute(options: BaseTransactionOptions<{
    proposalId: bigint;
}>): Promise<boolean>;
//# sourceMappingURL=canExecute.d.ts.map