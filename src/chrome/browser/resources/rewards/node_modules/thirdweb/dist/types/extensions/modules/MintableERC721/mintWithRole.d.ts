import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { NFTInput } from "../../../utils/nft/parseNft.js";
export type NFTMintParams = {
    to: string;
    nfts: (NFTInput | string)[];
};
/**
 * Mints ERC721 tokens to a specified address via a MintableERC721 module.
 * @param options The options for minting tokens.
 * @returns A transaction to mint tokens.
 * @example
 * ```typescript
 * import { MintableERC721 } from "thirdweb/modules";
 *
 * const transaction = MintableERC721.mintWithRole({
 *   contract,
 *   to: "0x...", // Address to mint tokens to
 *   nfts: [
 *     {
 *       name: "My NFT",
 *       description: "This is my NFT",
 *       image: "ipfs://...",
 *     },
 *   ],
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 * @modules MintableERC721
 */
export declare function mintWithRole(options: BaseTransactionOptions<NFTMintParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mintWithRole.d.ts.map