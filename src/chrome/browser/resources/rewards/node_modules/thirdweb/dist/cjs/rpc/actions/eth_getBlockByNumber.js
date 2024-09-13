"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_getBlockByNumber = eth_getBlockByNumber;
const viem_1 = require("viem");
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * Retrieves a block by its number or tag from the Ethereum blockchain.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the block.
 * @returns A promise that resolves to the requested block.
 * @throws An error if the block is not found.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getBlockByNumber } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const block = await eth_getBlockByNumber(rpcRequest, {
 *  blockNumber: 123456,
 *  includeTransactions: true,
 * });
 * ```
 */
async function eth_getBlockByNumber(request, params) {
    const blockTag = params.blockTag ?? "latest";
    const includeTransactions = params.includeTransactions ?? false;
    const blockNumberHex = params.blockNumber !== undefined
        ? (0, hex_js_1.numberToHex)(params.blockNumber)
        : undefined;
    const block = await request({
        method: "eth_getBlockByNumber",
        params: [blockNumberHex || blockTag, includeTransactions],
    });
    if (!block) {
        throw new Error("Block not found");
    }
    return (0, viem_1.formatBlock)(block);
}
//# sourceMappingURL=eth_getBlockByNumber.js.map