import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { DirectListing } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllValidListingParams = {
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
 * Retrieves all valid direct listings based on the provided options.
 * @param options - The options for retrieving the valid listing.
 * @returns A promise that resolves to the direct listings array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllValidListings } from "thirdweb/extensions/marketplace";
 *
 * const validListings = await getAllValidListings({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllValidListings(options: BaseTransactionOptions<GetAllValidListingParams>): Promise<DirectListing[]>;
//# sourceMappingURL=getAllValidListings.d.ts.map