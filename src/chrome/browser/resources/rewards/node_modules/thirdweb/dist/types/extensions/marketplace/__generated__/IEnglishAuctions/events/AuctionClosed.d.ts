import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AuctionClosed" event.
 */
export type AuctionClosedEventFilters = Partial<{
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
    closer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "closer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the AuctionClosed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { auctionClosedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  auctionClosedEvent({
 *  auctionId: ...,
 *  assetContract: ...,
 *  closer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function auctionClosedEvent(filters?: AuctionClosedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AuctionClosed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "auctionId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "closer";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
    }, {
        readonly type: "address";
        readonly name: "auctionCreator";
    }, {
        readonly type: "address";
        readonly name: "winningBidder";
    }];
}>;
//# sourceMappingURL=AuctionClosed.d.ts.map