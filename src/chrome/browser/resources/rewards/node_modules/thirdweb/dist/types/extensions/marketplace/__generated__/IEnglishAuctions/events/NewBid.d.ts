import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewBid" event.
 */
export type NewBidEventFilters = Partial<{
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "auctionId";
        indexed: true;
    }>;
    bidder: AbiParameterToPrimitiveType<{
        type: "address";
        name: "bidder";
        indexed: true;
    }>;
    assetContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "assetContract";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the NewBid event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { newBidEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  newBidEvent({
 *  auctionId: ...,
 *  bidder: ...,
 *  assetContract: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newBidEvent(filters?: NewBidEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewBid";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "auctionId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "bidder";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "bidAmount";
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "uint256";
            readonly name: "auctionId";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "quantity";
        }, {
            readonly type: "uint256";
            readonly name: "minimumBidAmount";
        }, {
            readonly type: "uint256";
            readonly name: "buyoutBidAmount";
        }, {
            readonly type: "uint64";
            readonly name: "timeBufferInSeconds";
        }, {
            readonly type: "uint64";
            readonly name: "bidBufferBps";
        }, {
            readonly type: "uint64";
            readonly name: "startTimestamp";
        }, {
            readonly type: "uint64";
            readonly name: "endTimestamp";
        }, {
            readonly type: "address";
            readonly name: "auctionCreator";
        }, {
            readonly type: "address";
            readonly name: "assetContract";
        }, {
            readonly type: "address";
            readonly name: "currency";
        }, {
            readonly type: "uint8";
            readonly name: "tokenType";
        }, {
            readonly type: "uint8";
            readonly name: "status";
        }];
        readonly name: "auction";
    }];
}>;
//# sourceMappingURL=NewBid.d.ts.map