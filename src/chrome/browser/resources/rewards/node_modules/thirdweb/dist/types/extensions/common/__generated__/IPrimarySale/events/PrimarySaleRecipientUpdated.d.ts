import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PrimarySaleRecipientUpdated" event.
 */
export type PrimarySaleRecipientUpdatedEventFilters = Partial<{
    recipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PrimarySaleRecipientUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { primarySaleRecipientUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  primarySaleRecipientUpdatedEvent({
 *  recipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function primarySaleRecipientUpdatedEvent(filters?: PrimarySaleRecipientUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PrimarySaleRecipientUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "recipient";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=PrimarySaleRecipientUpdated.d.ts.map