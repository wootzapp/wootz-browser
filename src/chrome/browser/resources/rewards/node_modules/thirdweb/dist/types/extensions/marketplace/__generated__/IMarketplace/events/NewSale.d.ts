import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewSale" event.
 */
export type NewSaleEventFilters = Partial<{
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
 * Creates an event object for the NewSale event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { newSaleEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  newSaleEvent({
 *  listingId: ...,
 *  assetContract: ...,
 *  lister: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newSaleEvent(filters?: NewSaleEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewSale";
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
        readonly type: "address";
        readonly name: "buyer";
    }, {
        readonly type: "uint256";
        readonly name: "quantityBought";
    }, {
        readonly type: "uint256";
        readonly name: "totalPricePaid";
    }];
}>;
//# sourceMappingURL=NewSale.d.ts.map