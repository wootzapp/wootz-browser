import type { Prettify } from "../../utils/type-utils.js";
import type { ClientAndChain } from "../../utils/types.js";
/**
 * @extension DEPLOY
 */
type DeployDetemisiticParams = Prettify<ClientAndChain & {
    contractId: string;
    constructorParams?: Record<string, unknown>;
    publisher?: string;
    version?: string;
    salt?: string;
}>;
/**
 * Deploy a contract deterministically - will maintain the same address across chains.
 * This is meant to be used with published contracts configured with the 'direct deploy' method.
 * Under the hood, this uses a keyless transaction with a common create2 factory.
 * @param options - the options to deploy the contract
 * @returns - the transaction to deploy the contract
 * @extension DEPLOY
 * @example
 * ```ts
 * import { prepareDeterministicDeployTransaction } from "thirdweb/deploys";
 * import { sepolia } from "thirdweb/chains";
 *
 * const tx = prepareDeterministicDeployTransaction({
 *  client,
 *  chain: sepolia,
 *  contractId: "AccountFactory",
 *  constructorParams: [123],
 * });
 * ```
 */
export declare function prepareDeterministicDeployTransaction(options: DeployDetemisiticParams): import("../../transaction/prepare-transaction.js").PreparedTransaction<[], import("abitype").AbiFunction, import("../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
export {};
//# sourceMappingURL=deploy-deterministic.d.ts.map