import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewListing" event.
 */
export type NewListingEventFilters = Partial<{
    listingCreator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "listingCreator";
        indexed: true;
    }>;
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "listingId";
        indexed: true;
    }>;
    assetContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "assetContract";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the NewListing event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { newListingEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  newListingEvent({
 *  listingCreator: ...,
 *  listingId: ...,
 *  assetContract: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newListingEvent(filters?: NewListingEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewListing";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "listingCreator";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "uint256";
            readonly name: "listingId";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "quantity";
        }, {
            readonly type: "uint256";
            readonly name: "pricePerToken";
        }, {
            readonly type: "uint128";
            readonly name: "startTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "endTimestamp";
        }, {
            readonly type: "address";
            readonly name: "listingCreator";
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
        }, {
            readonly type: "bool";
            readonly name: "reserved";
        }];
        readonly name: "listing";
    }];
}>;
//# sourceMappingURL=NewListing.d.ts.map