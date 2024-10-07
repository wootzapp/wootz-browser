/**
 * Creates an event object for the SetIdRegistry event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdRegistryEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdRegistryEvent()
 * ],
 * });
 * ```
 */
export declare function setIdRegistryEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetIdRegistry";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldIdRegistry";
    }, {
        readonly type: "address";
        readonly name: "newIdRegistry";
    }];
}>;
//# sourceMappingURL=SetIdRegistry.d.ts.map