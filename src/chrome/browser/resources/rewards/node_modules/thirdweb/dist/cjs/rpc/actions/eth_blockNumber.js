"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_blockNumber = eth_blockNumber;
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * Retrieves the current block number from the Ethereum blockchain.
 * @param request - The EIP1193 request function.
 * @returns A promise that resolves to the current block number as a bigint.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_blockNumber } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const blockNumber = await eth_blockNumber(rpcRequest);
 * ```
 */
async function eth_blockNumber(request) {
    const blockNumberHex = await request({
        method: "eth_blockNumber",
    });
    return (0, hex_js_1.hexToBigInt)(blockNumberHex);
}
//# sourceMappingURL=eth_blockNumber.js.map