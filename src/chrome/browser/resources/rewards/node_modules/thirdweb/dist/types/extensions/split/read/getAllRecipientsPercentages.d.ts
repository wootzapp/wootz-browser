import type { BaseTransactionOptions } from "../../../transaction/types.js";
import { type SplitRecipient } from "./getRecipientSplitPercentage.js";
/**
 * Get all the recipients of a Split contracts
 * @extension SPLIT
 * @returns an array of recipients' addresses and split percentage of each
 *
 * @example
 * ```ts
 * import { getAllRecipientsPercentages } from "thirdweb/extensions/split";
 *
 * const allRecipients = await getAllRecipientsPercentages({ contract });
 * // Example result:
 * [
 *   {
 *     address: "0x1...",
 *     splitPercentage: 25, // 25%
 *   },
 *   {
 *     address: "0x2...",
 *     splitPercentage: 75, // 75%
 *   },
 * ];
 * ```
 */
export declare function getAllRecipientsPercentages(options: BaseTransactionOptions): Promise<SplitRecipient[]>;
//# sourceMappingURL=getAllRecipientsPercentages.d.ts.map