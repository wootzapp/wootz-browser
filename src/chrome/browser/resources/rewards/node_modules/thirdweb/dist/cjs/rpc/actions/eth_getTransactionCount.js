"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_getTransactionCount = eth_getTransactionCount;
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * Retrieves the transaction count (nonce) for a given Ethereum address.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the transaction count.
 * @returns A promise that resolves to the transaction count as a number.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getTransactionCount } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const transactionCount = await eth_getTransactionCount(rpcRequest, {
 *  address: "0x...",
 * });
 * ```
 */
async function eth_getTransactionCount(request, params) {
    const count = await request({
        method: "eth_getTransactionCount",
        params: [
            params.address,
            // makes sense to default to `pending` here, since we're asking for a transaction count (nonce)
            params.blockNumber
                ? (0, hex_js_1.numberToHex)(params.blockNumber)
                : params.blockTag || "pending",
        ],
    });
    return (0, hex_js_1.hexToNumber)(count);
}
//# sourceMappingURL=eth_getTransactionCount.js.map