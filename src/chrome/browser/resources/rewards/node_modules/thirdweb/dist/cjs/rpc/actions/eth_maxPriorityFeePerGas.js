"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_maxPriorityFeePerGas = eth_maxPriorityFeePerGas;
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * Retrieves the maximum priority fee per gas from the Ethereum network.
 * @param request - The EIP1193 request function.
 * @returns A promise that resolves to a bigint representing the maximum priority fee per gas.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_maxPriorityFeePerGas } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const maxPriorityFeePerGas = await eth_maxPriorityFeePerGas(rpcRequest);
 * ```
 */
async function eth_maxPriorityFeePerGas(request) {
    const result = await request({
        method: "eth_maxPriorityFeePerGas",
    });
    return (0, hex_js_1.hexToBigInt)(result);
}
//# sourceMappingURL=eth_maxPriorityFeePerGas.js.map