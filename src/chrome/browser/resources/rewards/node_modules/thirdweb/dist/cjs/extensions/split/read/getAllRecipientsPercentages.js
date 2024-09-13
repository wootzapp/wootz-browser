"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getAllRecipientsPercentages = getAllRecipientsPercentages;
const getAllRecipientsAddresses_js_1 = require("./getAllRecipientsAddresses.js");
const getRecipientSplitPercentage_js_1 = require("./getRecipientSplitPercentage.js");
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
async function getAllRecipientsPercentages(options) {
    const { contract } = options;
    const recipientAddresses = await (0, getAllRecipientsAddresses_js_1.getAllRecipientsAddresses)({ contract });
    return await Promise.all(recipientAddresses.map((recipientAddress) => (0, getRecipientSplitPercentage_js_1.getRecipientSplitPercentage)({ contract, recipientAddress })));
}
//# sourceMappingURL=getAllRecipientsPercentages.js.map