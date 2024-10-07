"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getAllRecipientsAddresses = getAllRecipientsAddresses;
const payee_js_1 = require("../__generated__/Split/read/payee.js");
const payeeCount_js_1 = require("../__generated__/Split/read/payeeCount.js");
/**
 * Get the addresses of all recipients of a [`thirdweb Split contract`](https://thirdweb.com/thirdweb.eth/Split)
 * @extension SPLIT
 * @returns an array of wallet addresses
 *
 * @example
 * ```ts
 * import { getAllRecipientsAddresses } from "thirdweb/extensions/split";
 *
 * const addresses = await getAllRecipientsAddresses({ contract });
 */
async function getAllRecipientsAddresses(options) {
    const { contract } = options;
    const _totalRecipients = await (0, payeeCount_js_1.payeeCount)(options);
    const indexes = Array.from({ length: Number(_totalRecipients) }, (_, i) => i);
    const recipientAddresses = await Promise.all(indexes.map((index) => (0, payee_js_1.payee)({ contract, index: BigInt(index) })));
    return recipientAddresses;
}
//# sourceMappingURL=getAllRecipientsAddresses.js.map