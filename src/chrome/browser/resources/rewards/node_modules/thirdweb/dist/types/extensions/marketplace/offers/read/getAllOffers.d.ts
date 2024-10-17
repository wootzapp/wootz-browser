import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { Offer } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllOffersParams = {
    /**
     * The start index of the offers to retrieve.
     * @default 0
     */
    start?: number;
    /**
     * The number of offers to retrieve.
     * @default 100
     */
    count?: bigint;
};
/**
 * Retrieves all offers based on the provided options.
 * @param options - The options for retrieving the offers.
 * @returns A promise that resolves to the offers array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllOffers } from "thirdweb/extensions/marketplace";
 *
 * const listings = await getAllOffers({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllOffers(options: BaseTransactionOptions<GetAllOffersParams>): Promise<Offer[]>;
//# sourceMappingURL=getAllOffers.d.ts.map