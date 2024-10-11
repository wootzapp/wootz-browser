"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.currencyPriceForListing = currencyPriceForListing;
const isCurrencyApprovedForListing_js_1 = require("../../__generated__/IDirectListings/read/isCurrencyApprovedForListing.js");
const getListing_js_1 = require("./getListing.js");
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
async function currencyPriceForListing(options) {
    const [listing, isApprovedCurrency] = await Promise.all([
        (0, getListing_js_1.getListing)({
            contract: options.contract,
            listingId: options.listingId,
        }),
        (0, isCurrencyApprovedForListing_js_1.isCurrencyApprovedForListing)({
            contract: options.contract,
            currency: options.currency,
            listingId: options.listingId,
        }),
    ]);
    // if it's the same currency, return the price
    if (listing.currencyContractAddress === options.currency) {
        return listing.pricePerToken;
    }
    if (!isApprovedCurrency) {
        throw new Error(`Currency ${options.currency} is not approved for Listing ${options.listingId.toString()}.`);
    }
    const { currencyPriceForListing: generatedCurrencyPriceForListing } = await Promise.resolve().then(() => require("../../__generated__/IDirectListings/read/currencyPriceForListing.js"));
    return await generatedCurrencyPriceForListing({
        contract: options.contract,
        currency: options.currency,
        listingId: options.listingId,
    });
}
//# sourceMappingURL=currencyPriceForListing.js.map