import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { getListing } from "../__generated__/IDirectListings/read/getListing.js";
import type { DirectListing } from "./types.js";
/**
 * @internal
 */
export declare function mapDirectListing(options: BaseTransactionOptions<{
    latestBlock: {
        timestamp: bigint;
    };
    rawListing: Awaited<ReturnType<typeof getListing>>;
}>): Promise<DirectListing>;
type IsListingValidParams = {
    listing: DirectListing;
    quantity?: bigint;
};
type ValidReturn = {
    valid: true;
} | {
    valid: false;
    reason: string;
};
export declare function isListingValid(options: BaseTransactionOptions<IsListingValidParams>): Promise<ValidReturn>;
export {};
//# sourceMappingURL=utils.d.ts.map