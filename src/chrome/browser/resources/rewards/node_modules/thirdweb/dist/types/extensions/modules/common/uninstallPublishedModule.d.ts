import type { ThirdwebContract } from "../../../contract/contract.js";
/**
 * @modules
 */
export type UninstallPublishedModuleOptions = {
    contract: ThirdwebContract;
    moduleName: string;
    publisherAddress?: string;
    version?: string;
    moduleData?: `0x${string}`;
};
/**
 * Uninstall a published module
 * @param options - The options for uninstalling a published module
 * @returns A prepared transaction to send
 * @modules
 * @example
 * ```ts
 * import { uninstallPublishedModule } from "thirdweb/modules";
 * const transaction = uninstallPublishedModule({
 *  client,
 *  chain,
 *  contract,
 *  moduleName: "MyModule",
 *  publisherAddress: "0x...",
 * });
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function uninstallPublishedModule(options: UninstallPublishedModuleOptions): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=uninstallPublishedModule.d.ts.map