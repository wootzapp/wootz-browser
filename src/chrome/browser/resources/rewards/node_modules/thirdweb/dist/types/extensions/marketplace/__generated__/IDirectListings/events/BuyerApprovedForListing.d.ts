import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "BuyerApprovedForListing" event.
 */
export type BuyerApprovedForListingEventFilters = Partial<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "listingId";
        indexed: true;
    }>;
    buyer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "buyer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the BuyerApprovedForListing event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { buyerApprovedForListingEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  buyerApprovedForListingEvent({
 *  listingId: ...,
 *  buyer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function buyerApprovedForListingEvent(filters?: BuyerApprovedForListingEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "BuyerApprovedForListing";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "buyer";
        readonly indexed: true;
    }, {
        readonly type: "bool";
        readonly name: "approved";
    }];
}>;
//# sourceMappingURL=BuyerApprovedForListing.d.ts.map