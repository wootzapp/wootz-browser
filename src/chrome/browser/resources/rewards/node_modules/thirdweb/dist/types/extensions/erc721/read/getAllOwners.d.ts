import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Parameters for retrieving NFTs.
 * @extension ERC721
 */
export type GetAllOwnersParams = {
    /**
     * Which tokenId to start at.
     * @default 0
     */
    start?: number;
    /**
     * The number of NFTs to retrieve.
     * @default 100
     */
    count?: number;
};
/**
 * Retrieves the owners of all ERC721 tokens within a specified range.
 * @param options - The options for retrieving the owners.
 * @returns A promise that resolves to an array of objects containing the token ID and owner address.
 * @throws An error if the contract does not have either `nextTokenIdToMint` or `totalSupply` function available.
 * @extension ERC721
 * @example
 * ```ts
 * import { getAllOwners } from "thirdweb/extensions/erc721";
 * const owners = await getAllOwners({
 *  contract,
 *  start: 0,
 *  count: 10,
 * });
 * ```
 */
export declare function getAllOwners(options: BaseTransactionOptions<GetAllOwnersParams>): Promise<{
    tokenId: bigint;
    owner: string;
}[]>;
//# sourceMappingURL=getAllOwners.d.ts.map