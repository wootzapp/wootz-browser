import type { TransactionReceipt } from "../types.js";
import { type SendTransactionOptions } from "./send-transaction.js";
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
export declare function sendAndConfirmTransaction(options: SendTransactionOptions): Promise<TransactionReceipt>;
//# sourceMappingURL=send-and-confirm-transaction.d.ts.map