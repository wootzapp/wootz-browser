import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AcceptedOffer" event.
 */
export type AcceptedOfferEventFilters = Partial<{
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
    assetContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "assetContract";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the AcceptedOffer event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { acceptedOfferEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  acceptedOfferEvent({
 *  offeror: ...,
 *  offerId: ...,
 *  assetContract: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function acceptedOfferEvent(filters?: AcceptedOfferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AcceptedOffer";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "offeror";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "offerId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
    }, {
        readonly type: "address";
        readonly name: "seller";
    }, {
        readonly type: "uint256";
        readonly name: "quantityBought";
    }, {
        readonly type: "uint256";
        readonly name: "totalPricePaid";
    }];
}>;
//# sourceMappingURL=AcceptedOffer.d.ts.map