import { formatTransaction, } from "viem";
/**
 * Retrieves a transaction by its hash.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the transaction.
 * @returns A promise that resolves to the transaction.
 * @throws An error if the transaction is not found.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getTransactionByHash } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 *  const transaction = await eth_getTransactionByHash(rpcRequest, {
 *  hash: "0x...",
 * });
 * ```
 */
export async function eth_getTransactionByHash(request, params) {
    const receipt = await request({
        method: "eth_getTransactionByHash",
        params: [params.hash],
    });
    if (!receipt) {
        throw new Error("Transaction not found.");
    }
    return formatTransaction(receipt);
}
//# sourceMappingURL=eth_getTransactionByHash.js.map