import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewOffer" event.
 */
export type NewOfferEventFilters = Partial<{
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
 *  offeror: ...,
 *  offerId: ...,
 *  assetContract: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newOfferEvent(filters?: NewOfferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewOffer";
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
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "uint256";
            readonly name: "offerId";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "quantity";
        }, {
            readonly type: "uint256";
            readonly name: "totalPrice";
        }, {
            readonly type: "uint256";
            readonly name: "expirationTimestamp";
        }, {
            readonly type: "address";
            readonly name: "offeror";
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
        readonly name: "offer";
    }];
}>;
//# sourceMappingURL=NewOffer.d.ts.map