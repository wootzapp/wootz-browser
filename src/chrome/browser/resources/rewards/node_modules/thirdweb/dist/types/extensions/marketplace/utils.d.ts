import type { BaseTransactionOptions } from "../../transaction/types.js";
import type { NFT } from "../../utils/nft/parseNft.js";
import type { ListingStatus } from "./types.js";
/**
 * @extension MARKETPLACE
 */
type GetAssetParams = {
    tokenId: bigint;
};
/**
 * Retrieves the NFT asset based on the provided options.
 * @param options The transaction options.
 * @returns A promise that resolves to the NFT asset.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getNFTAsset } from "thirdweb/extensions/marketplace";
 *
 * const nft = await getNFTAsset({ contract, tokenId: 1n });
 * ```
 */
export declare function getNFTAsset(options: BaseTransactionOptions<GetAssetParams>): Promise<NFT>;
export declare function computeStatus(options: {
    listingStatus: number;
    blockTimeStamp: bigint;
    startTimestamp: bigint;
    endTimestamp: bigint;
}): ListingStatus;
/**
 * @internal
 */
export declare function getAllInBatches<const T>(fn: (start: bigint, end: bigint) => Promise<T>, options: {
    start: bigint;
    end: bigint;
    maxSize: bigint;
}): Promise<T[]>;
export {};
//# sourceMappingURL=utils.d.ts.map