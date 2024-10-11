import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewOffer" event.
 */
export type NewOfferEventFilters = Partial<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "listingId";
        indexed: true;
    }>;
    offeror: AbiParameterToPrimitiveType<{
        type: "address";
        name: "offeror";
        indexed: true;
    }>;
    listingType: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "listingType";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the NewOffer event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { newOfferEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  newOfferEvent({
 *  listingId: ...,
 *  offeror: ...,
 *  listingType: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newOfferEvent(filters?: NewOfferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewOffer";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "offeror";
        readonly indexed: true;
    }, {
        readonly type: "uint8";
        readonly name: "listingType";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "quantityWanted";
    }, {
        readonly type: "uint256";
        readonly name: "totalOfferAmount";
    }, {
        readonly type: "address";
        readonly name: "currency";
    }];
}>;
//# sourceMappingURL=NewOffer.d.ts.map