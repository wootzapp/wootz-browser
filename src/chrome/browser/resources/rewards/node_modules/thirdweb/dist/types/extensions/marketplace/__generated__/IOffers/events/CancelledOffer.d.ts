import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "CancelledOffer" event.
 */
export type CancelledOfferEventFilters = Partial<{
    offeror: AbiParameterToPrimitiveType<{
        type: "address";
        name: "offeror";
        indexed: true;
    }>;
    offerId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "offerId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the CancelledOffer event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { cancelledOfferEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  cancelledOfferEvent({
 *  offeror: ...,
 *  offerId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function cancelledOfferEvent(filters?: CancelledOfferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "CancelledOffer";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "offeror";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "offerId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=CancelledOffer.d.ts.map