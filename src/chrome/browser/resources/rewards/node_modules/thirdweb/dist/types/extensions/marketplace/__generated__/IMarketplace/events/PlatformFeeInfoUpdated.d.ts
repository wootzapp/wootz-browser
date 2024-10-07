import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PlatformFeeInfoUpdated" event.
 */
export type PlatformFeeInfoUpdatedEventFilters = Partial<{
    platformFeeRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "platformFeeRecipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PlatformFeeInfoUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { platformFeeInfoUpdatedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  platformFeeInfoUpdatedEvent({
 *  platformFeeRecipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function platformFeeInfoUpdatedEvent(filters?: PlatformFeeInfoUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PlatformFeeInfoUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "platformFeeRecipient";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "platformFeeBps";
    }];
}>;
//# sourceMappingURL=PlatformFeeInfoUpdated.d.ts.map