"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.sendAndConfirmTransaction = sendAndConfirmTransaction;
const send_transaction_js_1 = require("./send-transaction.js");
const wait_for_tx_receipt_js_1 = require("./wait-for-tx-receipt.js");
/**
 * Sends a transaction using the provided wallet.
 * @param options - The options for sending the transaction.
 * @returns A promise that resolves to the confirmed transaction receipt.
 * @throws An error if the wallet is not connected.
 * @transaction
 * @example
 *
 * ### Basic usage
 * ```ts
 * import { sendAndConfirmTransaction } from "thirdweb";
 *
 * const transactionReceipt = await sendAndConfirmTransaction({
 *  account,
 *  transaction
 * });
 * ```
 *
 * ### Gasless usage with [thirdweb Engine](https://portal.thirdweb.com/engine)
 * ```ts
 * const transactionReceipt = await sendAndConfirmTransaction({
 *  account,
 *  transaction,
 *  gasless: {
 *    provider: "engine",
 *    relayerUrl: "https://thirdweb.engine-***.thirdweb.com/relayer/***",
 *    relayerForwarderAddress: "0x...",
 *  }
 * });
 * ```
 *
 * ### Gasless usage with OpenZeppelin
 * ```ts
 * const transactionReceipt = await sendAndConfirmTransaction({
 *  account,
 *  transaction,
 *  gasless: {
 *    provider: "openzeppelin",
 *    relayerUrl: "https://...",
 *    relayerForwarderAddress: "0x...",
 *  }
 * });
 * ```
 */
async function sendAndConfirmTransaction(options) {
    const submittedTx = await (0, send_transaction_js_1.sendTransaction)(options);
    return (0, wait_for_tx_receipt_js_1.waitForReceipt)(submittedTx);
}
//# sourceMappingURL=send-and-confirm-transaction.js.map