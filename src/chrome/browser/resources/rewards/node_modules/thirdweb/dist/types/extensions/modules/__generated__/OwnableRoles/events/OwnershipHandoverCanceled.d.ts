import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "OwnershipHandoverCanceled" event.
 */
export type OwnershipHandoverCanceledEventFilters = Partial<{
    pendingOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "pendingOwner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the OwnershipHandoverCanceled event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ownershipHandoverCanceledEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ownershipHandoverCanceledEvent({
 *  pendingOwner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ownershipHandoverCanceledEvent(filters?: OwnershipHandoverCanceledEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "OwnershipHandoverCanceled";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "pendingOwner";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=OwnershipHandoverCanceled.d.ts.map