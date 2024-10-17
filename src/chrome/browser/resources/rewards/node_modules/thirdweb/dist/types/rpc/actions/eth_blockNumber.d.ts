import type { EIP1193RequestFn, EIP1474Methods } from "viem";
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
export declare function eth_blockNumber(request: EIP1193RequestFn<EIP1474Methods>): Promise<bigint>;
//# sourceMappingURL=eth_blockNumber.d.ts.map