import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { EnglishAuction } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllValidAuctionParams = {
    /**
     * The start index of the listings to retrieve.
     * @default 0
     */
    start?: number;
    /**
     * The number of listings to retrieve.
     * @default 100
     */
    count?: bigint;
};
/**
 * Retrieves all valid auctions based on the provided options.
 * @param options - The options for retrieving the listing.
 * @returns A promise that resolves to the valid auctions array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllValidAuctions } from "thirdweb/extensions/marketplace";
 *
 * const validAuctions = await getAllValidAuctions({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllValidAuctions(options: BaseTransactionOptions<GetAllValidAuctionParams>): Promise<EnglishAuction[]>;
//# sourceMappingURL=getAllValidAuctions.d.ts.map