import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { DirectListing } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllListingParams = {
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
 * Retrieves all direct listings based on the provided options.
 * @param options - The options for retrieving the listing.
 * @returns A promise that resolves to the direct listings array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllListings } from "thirdweb/extensions/marketplace";
 *
 * const listings = await getAllListings({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllListings(options: BaseTransactionOptions<GetAllListingParams>): Promise<DirectListing[]>;
//# sourceMappingURL=getAllListings.d.ts.map