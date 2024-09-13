import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type UpdateListingParams = {
    /**
     * The ID of the listing to update
     */
    listingId: bigint;
    /**
     * The contract address of the asset being listed
     */
    assetContractAddress?: Address;
    /**
     * The ID of the token being listed
     */
    tokenId?: bigint;
    /**
     * The quantity of tokens to list
     *
     * For ERC721s, this value can be omitted.
     * @default 1
     */
    quantity?: bigint;
    /**
     * The contract address of the currency to accept for the listing
     * @default NATIVE_TOKEN_ADDRESS
     */
    currencyContractAddress?: Address;
    /**
     * The start time of the listing
     * @default new Date()
     */
    startTimestamp?: Date;
    /**
     * The end time of the listing
     * @default new Date() + 10 years
     */
    endTimestamp?: Date;
    /**
     * Whether the listing is reserved to be bought from a specific set of buyers
     * @default false
     */
    isReservedListing?: boolean;
} & ({
    /**
     * The price per token (in Ether)
     */
    pricePerToken: string;
} | {
    /**
     * The price per token (in wei)
     */
    pricePerTokenWei: string;
});
/**
 * Updates an existing direct listing.
 * @param options The options for updating the direct listing.
 * @returns The result of updating the direct listing.
 * @extension MARKETPLACE
 * @example
 * ```typescript
 * import { updateListing } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = updateListing({...});
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function updateListing(options: BaseTransactionOptions<UpdateListingParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=updateListing.d.ts.map