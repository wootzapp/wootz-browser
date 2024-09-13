import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type IsBuyerApprovedForListingParams = {
    listingId: bigint;
    buyer: Address;
};
/**
 * Checks if the buyer is approved for a listing.
 *
 * @param options - The options for checking buyer approval.
 * @returns A promise that resolves to a boolean indicating whether the buyer is approved for the listing.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isBuyerApprovedForListing } from "thirdweb/extensions/marketplace";
 *
 * const isApproved = await isBuyerApprovedForListing({ contract, listingId: 1n, buyer: "0x..." });
 * ```
 */
export declare function isBuyerApprovedForListing(options: BaseTransactionOptions<IsBuyerApprovedForListingParams>): Promise<boolean>;
//# sourceMappingURL=isBuyerApprovedForListing.d.ts.map