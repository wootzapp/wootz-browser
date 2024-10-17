import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "CurrencyApprovedForListing" event.
 */
export type CurrencyApprovedForListingEventFilters = Partial<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "listingId";
        indexed: true;
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "currency";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the CurrencyApprovedForListing event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { currencyApprovedForListingEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  currencyApprovedForListingEvent({
 *  listingId: ...,
 *  currency: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function currencyApprovedForListingEvent(filters?: CurrencyApprovedForListingEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "CurrencyApprovedForListing";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "listingId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "currency";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "pricePerToken";
    }];
}>;
//# sourceMappingURL=CurrencyApprovedForListing.d.ts.map