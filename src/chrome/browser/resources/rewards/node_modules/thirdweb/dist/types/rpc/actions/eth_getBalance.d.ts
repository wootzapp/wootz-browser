import type { Address } from "abitype";
import type { BlockTag, EIP1193RequestFn, EIP1474Methods } from "viem";
type GetBalanceParams = {
    address: Address;
    blockTag?: BlockTag;
};
/**
 * Retrieves the balance of the specified Ethereum address.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the balance.
 * @returns A promise that resolves to the balance of the address in wei as bigint.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getBalance } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const balance = await eth_getBalance(rpcRequest, {
 *  address: "0x...",
 * });
 * ```
 */
export declare function eth_getBalance(request: EIP1193RequestFn<EIP1474Methods>, params: GetBalanceParams): Promise<bigint>;
export {};
//# sourceMappingURL=eth_getBalance.d.ts.map