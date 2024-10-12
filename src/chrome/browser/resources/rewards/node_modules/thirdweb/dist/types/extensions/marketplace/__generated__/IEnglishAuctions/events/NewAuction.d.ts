import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewAuction" event.
 */
export type NewAuctionEventFilters = Partial<{
    auctionCreator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "auctionCreator";
        indexed: true;
    }>;
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "auctionId";
        indexed: true;
    }>;
    assetContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "assetContract";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the NewAuction event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { newAuctionEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  newAuctionEvent({
 *  auctionCreator: ...,
 *  auctionId: ...,
 *  assetContract: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newAuctionEvent(filters?: NewAuctionEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewAuction";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "auctionCreator";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "auctionId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
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
//# sourceMappingURL=NewAuction.d.ts.map