import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RoyaltyEngineUpdated" event.
 */
export type RoyaltyEngineUpdatedEventFilters = Partial<{
    previousAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "previousAddress";
        indexed: true;
    }>;
    newAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newAddress";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RoyaltyEngineUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { royaltyEngineUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  royaltyEngineUpdatedEvent({
 *  previousAddress: ...,
 *  newAddress: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function royaltyEngineUpdatedEvent(filters?: RoyaltyEngineUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RoyaltyEngineUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "previousAddress";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "newAddress";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RoyaltyEngineUpdated.d.ts.map