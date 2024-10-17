import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type CurrencyPriceForListingParams = {
    listingId: bigint;
    currency: Address;
};
/**
 * Retrieves the currency price for a listing.
 *
 * @param options - The options for retrieving the currency price.
 * @returns A promise that resolves to the currency price as a bigint.
 * @throws An error if the currency is not approved for the listing.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { currencyPriceForListing } from "thirdweb/extensions/marketplace";
 *
 * const price = await currencyPriceForListing({ contract, listingId: 1n, currency: "0x..." });
 * ```
 */
export declare function currencyPriceForListing(options: BaseTransactionOptions<CurrencyPriceForListingParams>): Promise<bigint>;
//# sourceMappingURL=currencyPriceForListing.d.ts.map