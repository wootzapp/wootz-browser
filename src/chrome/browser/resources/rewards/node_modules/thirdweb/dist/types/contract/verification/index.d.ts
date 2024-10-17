import type { ThirdwebContract } from "../contract.js";
/**
 * @contract
 */
type VerifyContractOptions = {
    contract: ThirdwebContract;
    explorerApiUrl: string;
    explorerApiKey: string;
    encodedConstructorArgs?: string;
    type?: "etherscan" | "blockscoutV1" | "blockscoutV2" | "routescan";
};
/**
 * Verifies a contract by performing the following steps:
 * 1. Resolves the implementation of the contract.
 * 2. Extracts the IPFS URI from the contract bytecode.
 * 3. Downloads the contract source code from the IPFS URI.
 * 4. Fetches the source files from the compiler metadata.
 * 5. Compiles the contract source code using the Solidity compiler.
 * 6. Fetches the constructor parameters if not provided.
 * 7. Sends a request to the contract verification API to verify the contract source code.
 * @param options - The options for contract verification.
 * @returns A promise that resolves to the verification result.
 * @throws An error if any of the verification steps fail.
 * @example
 * ```ts
 * import { getContract } from "thirdweb/contract";
 * import { verifyContract } from "thirdweb/contract";
 *
 * const contract = getContract({ ... });
 * const verificationResult = await verifyContract({
 *  contract,
 *  explorerApiUrl: "https://api.polygonscan.com/api",
 *  explorerApiKey: "YOUR_API_KEY",
 * });
 * console.log(verificationResult);
 * ```
 * @contract
 */
export declare function verifyContract(options: VerifyContractOptions): Promise<string | string[]>;
type CheckVerificationStatusOptions = {
    explorerApiUrl: string;
    explorerApiKey: string;
    guid: string | string[];
};
/**
 * Checks the verification status of a contract.
 * @param options - The options for checking the verification status.
 * @returns A promise that resolves with the verification status data.
 * @throws An error if the verification status check fails.
 * @example
 * ```ts
 * import { checkVerificationStatus } from "thirdweb/contract";
 * const verificationStatus = await checkVerificationStatus({
 *  explorerApiUrl: "https://api.polygonscan.com/api",
 *  explorerApiKey: "YOUR_API_KEY",
 *  guid: "YOUR_GUID",
 * });
 * console.log(verificationStatus);
 * ```
 * @contract
 */
export declare function checkVerificationStatus(options: CheckVerificationStatusOptions): Promise<unknown>;
export {};
//# sourceMappingURL=index.d.ts.map