import { hexToBigInt } from "../../utils/encoding/hex.js";
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
export async function eth_getBalance(request, params) {
    const hexBalance = await request({
        method: "eth_getBalance",
        params: [params.address, params.blockTag || "latest"],
    });
    return hexToBigInt(hexBalance);
}
//# sourceMappingURL=eth_getBalance.js.map