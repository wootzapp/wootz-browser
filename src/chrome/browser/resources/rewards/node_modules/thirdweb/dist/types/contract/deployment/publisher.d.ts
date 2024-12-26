import type { ThirdwebClient } from "../../client/client.js";
import { type FetchDeployMetadataResult } from "../../utils/any-evm/deploy-metadata.js";
import { type ThirdwebContract } from "../contract.js";
export declare const CONTRACT_PUBLISHER_ADDRESS = "0xf5b896Ddb5146D5dA77efF4efBb3Eae36E300808";
/**
 * @internal
 */
export declare function fetchPublishedContractMetadata(options: {
    client: ThirdwebClient;
    contractId: string;
    publisher?: string;
    version?: string;
}): Promise<FetchDeployMetadataResult>;
/**
 *
 * @param contract
 * @example
 * @internal
 */
export declare function fetchDeployBytecodeFromPublishedContractMetadata(contract: ThirdwebContract): Promise<string | undefined>;
/**
 * @contract
 */
type FetchPublishedContractOptions = {
    publisherAddress: string;
    contractId: string;
    version?: string;
    client: ThirdwebClient;
};
/**
 * Fetches the published contract based on the provided options.
 * @param options - The options for fetching the published contract.
 * @returns The published contract.
 * @throws Error if the contract version or published contract is not found.
 * @example
 * ```ts
 * const publishedContract = await fetchPublishedContract({
 *  publisherAddress: "0x1234",
 *  contractName: "MyContract",
 *  version: "1.0.0",
 *  client: client,
 * });
 * ```
 * @contract
 */
export declare function fetchPublishedContract(options: FetchPublishedContractOptions): Promise<{
    contractId: string;
    publishTimestamp: bigint;
    publishMetadataUri: string;
    bytecodeHash: `0x${string}`;
    implementation: string;
}>;
export {};
//# sourceMappingURL=publisher.d.ts.map