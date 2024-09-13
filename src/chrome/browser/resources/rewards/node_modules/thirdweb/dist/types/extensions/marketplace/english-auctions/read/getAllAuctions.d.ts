import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { EnglishAuction } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllAuctionParams = {
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
 * Retrieves all auctions based on the provided options.
 * @param options - The options for retrieving the auctions.
 * @returns A promise that resolves to the auctions array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllAuctions } from "thirdweb/extensions/marketplace";
 *
 * const listings = await getAllAuctions({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllAuctions(options: BaseTransactionOptions<GetAllAuctionParams>): Promise<EnglishAuction[]>;
//# sourceMappingURL=getAllAuctions.d.ts.map