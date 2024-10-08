import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * @extension AIRDROP
 */
export type SaveSnapshotParams = {
    merkleRoot: string;
    snapshotUri: string;
};
/**
 * Generate merkle tree for a given snapshot and save the info on-chain.
 * @param options - The transaction options.
 * @example
 * ```ts
 *
 * // This is ERC20 example. Should import and use other ERC variants as needed
 *
 * import { generateMerkleTreeInfoERC20, saveSnapshot, setMerkleRoot } from "thirdweb/extensions/airdrop";
 *
 * // snapshot / allowlist of airdrop recipients and amounts
 * const snapshot = [
 *    { recipient: "0x...", amount: 10 },
 *    { recipient: "0x...", amount: 15 },
 *    { recipient: "0x...", amount: 20 },
 * ];
 *
 * const tokenAddress = "0x..." // Address of airdrop token
 *
 * const { merkleRoot, snapshotUri } = await generateMerkleTreeInfoERC20({
 *    contract,
 *    tokenAddress,
 *    snapshot
 * });
 *
 * const saveSnapshotTransaction = saveSnapshot({
 *   contract,
 *   merkleRoot,
 *   snapshotUri,
 * });
 * await sendTransaction({ saveSnapshotTransaction, account });
 *
 * const setMerkleRootTransaction = setMerkleRoot({
 *   contract,
 *   token,
 *   tokenMerkleRoot: merkleRoot as `0x${string}`,
 *   resetClaimStatus: false // toggle as needed
 *   signature,
 * });
 * await sendTransaction({ setMerkleRootTransaction, account });
 *
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the transaction result.
 */
export declare function saveSnapshot(options: BaseTransactionOptions<SaveSnapshotParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=saveSnapshot.d.ts.map