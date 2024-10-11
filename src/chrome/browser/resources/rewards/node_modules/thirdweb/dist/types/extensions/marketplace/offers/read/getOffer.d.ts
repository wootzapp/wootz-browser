import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import { type GetOfferParams as GetOfferParamsGenerated } from "../../__generated__/IOffers/read/getOffer.js";
import type { Offer } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetOfferParams = GetOfferParamsGenerated;
/**
 * Retrieves an offer based on the provided options.
 * @param options - The options for retrieving the offer.
 * @returns A promise that resolves to the offer.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getOffer } from "thirdweb/extensions/marketplace";
 *
 * const listing = await getOffer({ contract, listingId: 1n });
 * ```
 */
export declare function getOffer(options: BaseTransactionOptions<GetOfferParams>): Promise<Offer>;
//# sourceMappingURL=getOffer.d.ts.map