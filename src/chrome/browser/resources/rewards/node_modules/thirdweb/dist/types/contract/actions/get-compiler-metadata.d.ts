import type { ThirdwebContract } from "../contract.js";
/**
 * Down the compiled metadata from thirdweb contract api and format it
 * @param metadata The (json) data returned from https://contract.thirdweb.com/metadata/<chainId>/<contractAddress>
 *
 * @example
 * ```ts
 * import { getCompilerMetadata, getContract } from "thirdweb/contracts";
 *
 * const contract = getContract({
 *   address: "0x...",
 *   chain: ethereum,
 *   client: "",
 * });
 * const metadata = await getCompilerMetadata(contract);
 * ```
 * @returns The compiler metadata for the contract
 * @contract
 */
export declare function getCompilerMetadata(contract: ThirdwebContract): Promise<import("./compiler-metadata.js").CompilerMetadata>;
//# sourceMappingURL=get-compiler-metadata.d.ts.map