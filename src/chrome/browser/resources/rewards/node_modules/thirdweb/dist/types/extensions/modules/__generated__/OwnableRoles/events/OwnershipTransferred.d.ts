import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "OwnershipTransferred" event.
 */
export type OwnershipTransferredEventFilters = Partial<{
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
 * Creates an event object for the OwnershipTransferred event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ownershipTransferredEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ownershipTransferredEvent({
 *  oldOwner: ...,
 *  newOwner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ownershipTransferredEvent(filters?: OwnershipTransferredEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "OwnershipTransferred";
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
//# sourceMappingURL=OwnershipTransferred.d.ts.map