"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_getTransactionReceipt = eth_getTransactionReceipt;
const viem_1 = require("viem");
/**
 * Retrieves the transaction receipt for a given transaction hash.
 * Throws an error if the receipt is not found.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the transaction receipt.
 * @returns A promise that resolves to the transaction receipt.
 * @throws An error if the transaction receipt is not found.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getTransactionReceipt } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const transactionReceipt = await eth_getTransactionReceipt(rpcRequest, {
 *  hash: "0x...",
 * });
 * ```
 */
async function eth_getTransactionReceipt(request, params) {
    const receipt = await request({
        method: "eth_getTransactionReceipt",
        params: [params.hash],
    });
    if (!receipt) {
        throw new Error("Transaction receipt not found.");
    }
    return (0, viem_1.formatTransactionReceipt)(receipt);
}
//# sourceMappingURL=eth_getTransactionReceipt.js.map