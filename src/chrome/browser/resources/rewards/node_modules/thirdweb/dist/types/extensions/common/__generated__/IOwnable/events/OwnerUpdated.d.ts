import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "OwnerUpdated" event.
 */
export type OwnerUpdatedEventFilters = Partial<{
    prevOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "prevOwner";
        indexed: true;
    }>;
    newOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newOwner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the OwnerUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ownerUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ownerUpdatedEvent({
 *  prevOwner: ...,
 *  newOwner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ownerUpdatedEvent(filters?: OwnerUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "OwnerUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "prevOwner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "newOwner";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=OwnerUpdated.d.ts.map