import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * @extension SPLIT
 */
export interface SplitRecipient {
    /**
     * The address of the recipient
     */
    address: string;
    /**
     * The split of the recipient as a percentage of the total amount
     *
     * I.e. If a recipient has a split of 50%, and the asset sells for 100 ETH,
     * the recipient will receive 50 ETH.
     */
    splitPercentage: number;
}
/**
 * Get the split percentage of a recipient
 * @param options - The options for the transaction
 * @param options.recipientAddress - The address of the recipient
 * @returns The split percentage of the recipient
 * @extension SPLIT
 * @example
 * ```ts
 * import { getRecipientSplitPercentage } from "thirdweb/extensions/split";
 *
 * const percentage = await getRecipientSplitPercentage({ recipientAddress: "0x..." });
 * ```
 */
export declare function getRecipientSplitPercentage(options: BaseTransactionOptions<{
    recipientAddress: string;
}>): Promise<SplitRecipient>;
//# sourceMappingURL=getRecipientSplitPercentage.d.ts.map