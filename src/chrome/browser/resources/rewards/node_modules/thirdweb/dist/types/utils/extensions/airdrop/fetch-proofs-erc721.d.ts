import type { ThirdwebContract } from "../../../contract/contract.js";
import type { ClaimProofERC721 } from "./types.js";
/**
 * Retrieves the claim merkle proof for the provided address.
 * @param {Object} options
 * @param {@link ThirdwebContract} contract - The ERC721 airdrop contract
 * @param {string} recipient - The address of the drop recipient
 * @param {string} merkleRoot - The merkle root, found on the active claim condition
 *
 * @returns {Promise<ClaimProofERC721 | null>} A promise that resolves to the proof or null if the recipient is not in the allowlist
 *
 * @example
 * ```ts
 * import { fetchProofsERC721 } from "thirdweb/extensions/airdrop";
 * import { getContract, defineChain } from "thirdweb";
 *
 * const NFT = getContracct({
 *  client,
 *  chain: defineChain(1),
 *  address: "0x..."
 * });
 *
 * const merkleRoot = await tokenMerkleRoot({
 *  contract: NFT,
 *  tokenAddress: NFT.address
 * });
 *
 * const proof = await fetchProofsERC721({
 *  contract: NFT,
 *  recipient: "0x...",
 *  merkleRoot
 * });
 * ```
 *
 * @extension AIRDROP
 */
export declare function fetchProofsERC721(options: {
    contract: ThirdwebContract;
    recipient: string;
    merkleRoot: string;
}): Promise<ClaimProofERC721 | null>;
//# sourceMappingURL=fetch-proofs-erc721.d.ts.map