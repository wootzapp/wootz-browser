import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ListingUpdated" event.
 */
export type ListingUpdatedEventFilters = Partial<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "listingId";
        indexed: true;
    }>;
    listingCreator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "listingCreator";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ListingUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { listingUpdatedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  listingUpdatedEvent({
 *  listingId: ...,
 *  listingCreator: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function listingUpdatedEvent(filters?: ListingUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ListingUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "listingCreator";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ListingUpdated.d.ts.map