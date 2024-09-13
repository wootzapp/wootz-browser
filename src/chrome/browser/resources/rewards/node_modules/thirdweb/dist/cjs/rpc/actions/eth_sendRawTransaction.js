"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_sendRawTransaction = eth_sendRawTransaction;
/**
 * Sends a raw transaction to the Ethereum network.
 * @param request - The EIP1193 request function.
 * @param signedTransaction - The signed transaction in hex format.
 * @returns A promise that resolves to the transaction hash.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_sendRawTransaction } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const transactionHash = await eth_sendRawTransaction(rpcRequest, "0x...");
 * ```
 */
async function eth_sendRawTransaction(request, signedTransaction) {
    return await request({
        method: "eth_sendRawTransaction",
        params: [signedTransaction],
    });
}
//# sourceMappingURL=eth_sendRawTransaction.js.map