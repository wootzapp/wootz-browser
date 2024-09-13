"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.eth_getBlockByHash = eth_getBlockByHash;
const viem_1 = require("viem");
/**
 * Retrieves a block by its hash.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for the block retrieval.
 * @returns A promise that resolves to the retrieved block.
 * @throws An error if the block is not found.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getBlockByHash } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const block = await eth_getBlockByHash(rpcRequest, {
 * blockHash: "0x...",
 * includeTransactions: true,
 * });
 * ```
 */
async function eth_getBlockByHash(request, params) {
    const includeTransactions = params.includeTransactions ?? false;
    const block = await request({
        method: "eth_getBlockByHash",
        params: [params.blockHash, includeTransactions],
    });
    if (!block) {
        throw new Error("Block not found");
    }
    return (0, viem_1.formatBlock)(block);
}
//# sourceMappingURL=eth_getBlockByHash.js.map