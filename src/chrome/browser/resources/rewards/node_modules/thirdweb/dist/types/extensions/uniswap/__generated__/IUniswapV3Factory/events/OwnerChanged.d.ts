import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "OwnerChanged" event.
 */
export type OwnerChangedEventFilters = Partial<{
    oldOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "oldOwner";
        indexed: true;
    }>;
    newOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newOwner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the OwnerChanged event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ownerChangedEvent } from "thirdweb/extensions/uniswap";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ownerChangedEvent({
 *  oldOwner: ...,
 *  newOwner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ownerChangedEvent(filters?: OwnerChangedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "OwnerChanged";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldOwner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "newOwner";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=OwnerChanged.d.ts.map