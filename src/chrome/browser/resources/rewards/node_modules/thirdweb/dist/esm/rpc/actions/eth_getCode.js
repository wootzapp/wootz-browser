/**
 * Retrieves the bytecode of a smart contract at the specified address.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for the eth_getCode method.
 * @returns A promise that resolves to the bytecode of the smart contract.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getCode } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const bytecode = await eth_getCode(rpcRequest, {
 *  address: "0x...",
 * });
 * ```
 */
export async function eth_getCode(request, params) {
    return request({
        method: "eth_getCode",
        params: [params.address, params.blockTag || "latest"],
    });
}
//# sourceMappingURL=eth_getCode.js.map