import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import type { Offer } from "../types.js";
/**
 * @extension MARKETPLACE
 */
export type GetAllValidOffersParams = {
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
 * Retrieves all valid offers based on the provided options.
 * @param options - The options for retrieving the valid offers.
 * @returns A promise that resolves to the offers array.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getAllValidOffers } from "thirdweb/extensions/marketplace";
 *
 * const validOffers = await getAllValidOffers({ contract, start: 0, count: 10 });
 * ```
 */
export declare function getAllValidOffers(options: BaseTransactionOptions<GetAllValidOffersParams>): Promise<Offer[]>;
//# sourceMappingURL=getAllValidOffers.d.ts.map