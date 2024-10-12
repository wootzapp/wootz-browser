import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { getOffer } from "../__generated__/IOffers/read/getOffer.js";
import type { Offer } from "./types.js";
/**
 * @internal
 */
export declare function mapOffer(options: BaseTransactionOptions<{
    latestBlock: {
        timestamp: bigint;
    };
    rawOffer: Awaited<ReturnType<typeof getOffer>>;
}>): Promise<Offer>;
//# sourceMappingURL=utils.d.ts.map