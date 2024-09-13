/**
 * Extracts the IPFS URI from the given bytecode.
 * @param bytecode - The bytecode to extract the IPFS URI from.
 * @returns The IPFS URI if found, otherwise undefined.
 * @example
 * ```ts
 * import { extractIPFSUri } from "thirdweb/utils/bytecode/extractIPFS";
 * const bytecode = "0x363d3d373d3d3d363d30545af43d82803e903d91601857fd5bf3";
 * const ipfsHash = extractIPFSUri(bytecode);
 * console.log(ipfsHash);
 * ```
 * @utils
 */
export declare function extractIPFSUri(bytecode: string): string | undefined;
//# sourceMappingURL=extractIPFS.d.ts.map