import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
type MinimumBidAmount = {
    /**
     * The minimum price that a bid must be in order to be accepted. (in Ether)
     */
    minimumBidAmount: string;
} | {
    /**
     * The minimum price that a bid must be in order to be accepted. (in wei)
     */
    minimumBidAmountWei: bigint;
};
/**
 * @extension MARKETPLACE
 */
type BuyoutBidAmount = {
    /**
     * The buyout price of the auction. (in Ether)
     */
    buyoutBidAmount: string;
} | {
    /**
     * The buyout price of the auction. (in wei)
     */
    buyoutBidAmountWei: bigint;
};
/**
 * @extension MARKETPLACE
 */
export type CreateAuctionParams = {
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
     * This is a buffer e.g. x seconds.
     *
     * If a new winning bid is made less than x seconds before expirationTimestamp, the
     * expirationTimestamp is increased by x seconds.
     * @default 900 (15 minutes)
     */
    timeBufferInSeconds?: number;
    /**
     * This is a buffer in basis points e.g. x%.
     *
     * To be considered as a new winning bid, a bid must be at least x% greater than
     * the current winning bid.
     * @default 500 (5%)
     */
    bidBufferBps?: number;
} & MinimumBidAmount & BuyoutBidAmount;
/**
 * Creates an auction.
 * @param options The options for creating the auction.
 * @returns The result of creating the auction.
 * @extension MARKETPLACE
 * @example
 * ```typescript
 * import { createAuction } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = createAuction({...});
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createAuction(options: BaseTransactionOptions<CreateAuctionParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
/**
 * Checks if the `createAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCreateAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCreateAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCreateAuctionSupported(availableSelectors: string[]): boolean;
export {};
//# sourceMappingURL=createAuction.d.ts.map