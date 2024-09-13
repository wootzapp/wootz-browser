import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "CancelledAuction" event.
 */
export type CancelledAuctionEventFilters = Partial<{
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
}>;
/**
 * Creates an event object for the CancelledAuction event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { cancelledAuctionEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  cancelledAuctionEvent({
 *  auctionCreator: ...,
 *  auctionId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function cancelledAuctionEvent(filters?: CancelledAuctionEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "CancelledAuction";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "auctionCreator";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "auctionId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=CancelledAuction.d.ts.map