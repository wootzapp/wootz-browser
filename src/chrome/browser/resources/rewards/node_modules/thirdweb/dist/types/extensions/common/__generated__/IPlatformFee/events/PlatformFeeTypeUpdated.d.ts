/**
 * Creates an event object for the PlatformFeeTypeUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { platformFeeTypeUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  platformFeeTypeUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function platformFeeTypeUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PlatformFeeTypeUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint8";
        readonly name: "feeType";
    }];
}>;
//# sourceMappingURL=PlatformFeeTypeUpdated.d.ts.map