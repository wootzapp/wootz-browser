import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { SnapshotEntryERC721 } from "../../../utils/extensions/airdrop/types.js";
/**
 * @extension AIRDROP
 */
export type GenerateMerkleTreeInfoERC721Params = {
    snapshot: SnapshotEntryERC721[];
    tokenAddress: string;
};
/**
 * Generate merkle tree for a given snapshot.
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { generateMerkleTreeInfoERC721 } from "thirdweb/extensions/airdrop";
 *
 * // snapshot / allowlist of airdrop recipients and amounts
 * const snapshot = [
 *    { recipient: "0x...", tokenId: 0 },
 *    { recipient: "0x...", tokenId: 1 },
 *    { recipient: "0x...", tokenId: 2 },
 * ];
 *
 * const tokenAddress = "0x..." // Address of ERC721 airdrop token
 *
 * const { merkleRoot, snapshotUri } = await generateMerkleTreeInfoERC721({
 *    contract,
 *    tokenAddress,
 *    snapshot
 * });
 *
 * // Optional next steps {See: saveSnapshot and setMerkleRoot functions}
 * // - Save snapshot on-chain (on the airdrop contract uri)
 * // - Set merkle root on the contract to enable claiming
 *
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the merkle-root and snapshot-uri.
 */
export declare function generateMerkleTreeInfoERC721(options: BaseTransactionOptions<GenerateMerkleTreeInfoERC721Params>): Promise<{
    merkleRoot: string;
    snapshotUri: string;
}>;
//# sourceMappingURL=merkleInfoERC721.d.ts.map