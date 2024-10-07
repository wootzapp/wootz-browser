import type { ThirdwebContract } from "../../../contract/contract.js";
import type { ClaimProofERC20 } from "./types.js";
/**
 * Retrieves the claim merkle proof for the provided address.
 * @param {Object} options
 * @param {@link ThirdwebContract} contract - The ERC20 airdrop contract
 * @param {string} recipient - The address of the drop recipient
 * @param {string} merkleRoot - The merkle root, found on the active claim condition
 *
 * @returns {Promise<ClaimProofERC20 | null>} A promise that resolves to the proof or null if the recipient is not in the allowlist
 *
 * @example
 * ```ts
 * import { fetchProofsERCC20 } from "thirdweb/extensions/airdrop";
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
 * const proof = await fetchProofsERC20({
 *  contract: TOKEN,
 *  recipient: "0x...",
 *  merkleRoot
 * });
 * ```
 *
 * @extension AIRDROP
 */
export declare function fetchProofsERC20(options: {
    contract: ThirdwebContract;
    recipient: string;
    merkleRoot: string;
    tokenDecimals: number;
}): Promise<ClaimProofERC20 | null>;
//# sourceMappingURL=fetch-proofs-erc20.d.ts.map