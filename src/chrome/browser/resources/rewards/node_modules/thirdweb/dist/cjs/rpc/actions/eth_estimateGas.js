"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_estimateGas = eth_estimateGas;
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * Estimates the gas required to execute a transaction on the Ethereum network.
 * @param request - The EIP1193 request function.
 * @param transactionRequest - The transaction request object.
 * @returns A promise that resolves to the estimated gas as a bigint.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_estimateGas } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const gas = await eth_estimateGas(rpcRequest, {
 *  to: "0x...",
 *  ...
 * });
 * ```
 */
async function eth_estimateGas(request, transactionRequest) {
    const estimateResult = await request({
        method: "eth_estimateGas",
        params: [transactionRequest],
    });
    return (0, hex_js_1.hexToBigInt)(estimateResult);
}
//# sourceMappingURL=eth_estimateGas.js.map