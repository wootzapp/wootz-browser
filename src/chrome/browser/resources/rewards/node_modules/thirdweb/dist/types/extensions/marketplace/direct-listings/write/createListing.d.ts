import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type CreateListingParams = {
    /**
     * The contract address of the asset being listed
     */
    assetContractAddress: Address;
    /**
     * The ID of the token being listed
     */
    tokenId: bigint;
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
 * Creates a direct listing.
 * @param options The options for creating the direct listing.
 * @returns The result of creating the direct listing.
 * @extension MARKETPLACE
 * @example
 * ```typescript
 * import { createListing } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = createListing({
 *   assetContractAddress: "0x...", // the NFT contract address that you want to sell
 *   tokenId={0n}, // the token id you want to sell
 *   pricePerToken="0.1" // sell for 0.1 <native token>
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createListing(options: BaseTransactionOptions<CreateListingParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
/**
 * Checks if the `createListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCreateAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCreateAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCreateListingSupported(availableSelectors: string[]): boolean;
//# sourceMappingURL=createListing.d.ts.map