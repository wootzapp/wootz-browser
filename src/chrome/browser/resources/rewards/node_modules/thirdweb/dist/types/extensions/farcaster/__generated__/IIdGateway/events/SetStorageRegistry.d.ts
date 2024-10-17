/**
 * Creates an event object for the SetStorageRegistry event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setStorageRegistryEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setStorageRegistryEvent()
 * ],
 * });
 * ```
 */
export declare function setStorageRegistryEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetStorageRegistry";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldStorageRegistry";
    }, {
        readonly type: "address";
        readonly name: "newStorageRegistry";
    }];
}>;
//# sourceMappingURL=SetStorageRegistry.d.ts.map