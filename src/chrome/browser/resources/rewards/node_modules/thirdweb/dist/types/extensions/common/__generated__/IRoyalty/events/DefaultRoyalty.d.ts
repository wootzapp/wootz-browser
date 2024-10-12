import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "DefaultRoyalty" event.
 */
export type DefaultRoyaltyEventFilters = Partial<{
    newRoyaltyRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newRoyaltyRecipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the DefaultRoyalty event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { defaultRoyaltyEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  defaultRoyaltyEvent({
 *  newRoyaltyRecipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function defaultRoyaltyEvent(filters?: DefaultRoyaltyEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "DefaultRoyalty";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "newRoyaltyRecipient";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "newRoyaltyBps";
    }];
}>;
//# sourceMappingURL=DefaultRoyalty.d.ts.map