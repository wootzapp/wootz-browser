import type { ThirdwebContract } from "../../../contract/contract.js";
import type { ClaimProofERC1155 } from "./types.js";
/**
 * Retrieves the claim merkle proof for the provided address.
 * @param {Object} options
 * @param {@link ThirdwebContract} contract - The ERC1155 airdrop contract
 * @param {string} recipient - The address of the drop recipient
 * @param {string} merkleRoot - The merkle root, found on the active claim condition
 *
 * @returns {Promise<ClaimProofERC20 | null>} A promise that resolves to the proof or null if the recipient is not in the allowlist
 *
 * @example
 * ```ts
 * import { fetchProofsERCC1155 } from "thirdweb/extensions/airdrop";
 * import { getContract, defineChain } from "thirdweb";
 *
 * const TOKEN = getContracct({
 *  client,
 *  chain: defineChain(1),
 *  address: "0x..."
 * });
 *
 * const merkleRoot = await tokenMerkleRoot({
 *  contract: TOKEN,
 *  tokenAddress: TOKEN.address
 * });
 *
 * const proof = await fetchProofsERC1155({
 *  contract: TOKEN,
 *  recipient: "0x...",
 *  merkleRoot
 * });
 * ```
 *
 * @extension AIRDROP
 */
export declare function fetchProofsERC1155(options: {
    contract: ThirdwebContract;
    recipient: string;
    merkleRoot: string;
}): Promise<ClaimProofERC1155 | null>;
//# sourceMappingURL=fetch-proofs-erc1155.d.ts.map