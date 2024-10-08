import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { SnapshotEntryERC20 } from "../../../utils/extensions/airdrop/types.js";
/**
 * @extension AIRDROP
 */
export type GenerateMerkleTreeInfoERC20Params = {
    snapshot: SnapshotEntryERC20[];
    tokenAddress: string;
};
/**
 * Generate merkle tree for a given snapshot.
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { generateMerkleTreeInfoERC20 } from "thirdweb/extensions/airdrop";
 *
 * // snapshot / allowlist of airdrop recipients and amounts
 * const snapshot = [
 *    { recipient: "0x...", amount: 10 },
 *    { recipient: "0x...", amount: 15 },
 *    { recipient: "0x...", amount: 20 },
 * ];
 *
 * const tokenAddress = "0x..." // Address of ERC20 airdrop token
 *
 * const { merkleRoot, snapshotUri } = await generateMerkleTreeInfoERC20({
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
export declare function generateMerkleTreeInfoERC20(options: BaseTransactionOptions<GenerateMerkleTreeInfoERC20Params>): Promise<{
    merkleRoot: string;
    snapshotUri: string;
}>;
//# sourceMappingURL=merkleInfoERC20.d.ts.map