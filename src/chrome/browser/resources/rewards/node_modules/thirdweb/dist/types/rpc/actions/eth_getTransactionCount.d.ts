import type { EIP1193RequestFn, EIP1474Methods, GetTransactionCountParameters } from "viem";
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
export declare function eth_getTransactionCount(request: EIP1193RequestFn<EIP1474Methods>, params: GetTransactionCountParameters): Promise<number>;
//# sourceMappingURL=eth_getTransactionCount.d.ts.map