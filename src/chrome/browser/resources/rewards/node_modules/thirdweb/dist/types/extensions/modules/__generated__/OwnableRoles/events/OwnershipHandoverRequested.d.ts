import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "OwnershipHandoverRequested" event.
 */
export type OwnershipHandoverRequestedEventFilters = Partial<{
    pendingOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "pendingOwner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the OwnershipHandoverRequested event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ownershipHandoverRequestedEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ownershipHandoverRequestedEvent({
 *  pendingOwner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ownershipHandoverRequestedEvent(filters?: OwnershipHandoverRequestedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "OwnershipHandoverRequested";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "pendingOwner";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=OwnershipHandoverRequested.d.ts.map