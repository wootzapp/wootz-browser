import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ListingAdded" event.
 */
export type ListingAddedEventFilters = Partial<{
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
    lister: AbiParameterToPrimitiveType<{
        type: "address";
        name: "lister";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ListingAdded event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { listingAddedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  listingAddedEvent({
 *  listingId: ...,
 *  assetContract: ...,
 *  lister: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function listingAddedEvent(filters?: ListingAddedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ListingAdded";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "assetContract";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "lister";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "uint256";
            readonly name: "listingId";
        }, {
            readonly type: "address";
            readonly name: "tokenOwner";
        }, {
            readonly type: "address";
            readonly name: "assetContract";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "startTime";
        }, {
            readonly type: "uint256";
            readonly name: "endTime";
        }, {
            readonly type: "uint256";
            readonly name: "quantity";
        }, {
            readonly type: "address";
            readonly name: "currency";
        }, {
            readonly type: "uint256";
            readonly name: "reservePricePerToken";
        }, {
            readonly type: "uint256";
            readonly name: "buyoutPricePerToken";
        }, {
            readonly type: "uint8";
            readonly name: "tokenType";
        }, {
            readonly type: "uint8";
            readonly name: "listingType";
        }];
        readonly name: "listing";
    }];
}>;
//# sourceMappingURL=ListingAdded.d.ts.map