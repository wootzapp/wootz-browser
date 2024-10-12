import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Parameters for retrieving owned tokenIds of an ERC1155 contract.
 * @extension ERC1155
 */
export type GetOwnedTokenIdsParams = {
    /**
     * Which tokenId to start at.
     */
    start?: number;
    /**
     * The number of NFTs to retrieve.
     */
    count?: number;
    /**
     * The address of the wallet to get the NFTs of.
     */
    address: string;
};
/**
 * Retrieves the owned ERC1155 tokenIds & the owned balance of each tokenId for a given wallet address.
 * @param options - The transaction options and parameters.
 * @returns A promise that resolves to an array of ERC1155 NFTs owned by the wallet address, along with the quantity owned.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getOwnedNFTs } from "thirdweb/extensions/erc1155";
 * const ownedTokenIds = await getOwnedTokenIds({
 *  contract,
 *  start: 0,
 *  count: 10,
 *  address: "0x123...",
 * });
 * ```
 */
export declare function getOwnedTokenIds(options: BaseTransactionOptions<GetOwnedTokenIdsParams>): Promise<{
    tokenId: bigint;
    balance: bigint;
}[]>;
//# sourceMappingURL=getOwnedTokenIds.d.ts.map