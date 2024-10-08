import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import { type ThirdwebContract } from "../../../contract/contract.js";
/**
 * @modules
 */
export type UninstallModuleByProxyOptions = {
    client: ThirdwebClient;
    chain: Chain;
    contract: ThirdwebContract;
    moduleProxyAddress: string;
    moduleData?: `0x${string}`;
};
/**
 * Uninstall an module by proxy
 * @param options - The options for uninstalling an module by proxy
 * @returns A prepared transaction to send
 * @modules
 * @example
 * ```ts
 * import { uninstallModuleByProxy } from "thirdweb/modules";
 * const transaction = uninstallModuleByProxy({
 *  client,
 *  chain,
 *  contract,
 *  moduleProxyAddress: "0x...",
 * });
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function uninstallModuleByProxy(options: UninstallModuleByProxyOptions): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=uninstallModuleByProxy.d.ts.map