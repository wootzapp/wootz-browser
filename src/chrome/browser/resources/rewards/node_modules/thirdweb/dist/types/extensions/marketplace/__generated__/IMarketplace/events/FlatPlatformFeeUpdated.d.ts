/**
 * Creates an event object for the FlatPlatformFeeUpdated event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { flatPlatformFeeUpdatedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  flatPlatformFeeUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function flatPlatformFeeUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "FlatPlatformFeeUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "platformFeeRecipient";
    }, {
        readonly type: "uint256";
        readonly name: "flatFee";
    }];
}>;
//# sourceMappingURL=FlatPlatformFeeUpdated.d.ts.map