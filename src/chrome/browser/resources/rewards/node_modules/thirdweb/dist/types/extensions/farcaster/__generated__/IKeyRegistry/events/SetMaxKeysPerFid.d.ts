/**
 * Creates an event object for the SetMaxKeysPerFid event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setMaxKeysPerFidEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setMaxKeysPerFidEvent()
 * ],
 * });
 * ```
 */
export declare function setMaxKeysPerFidEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetMaxKeysPerFid";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldMax";
    }, {
        readonly type: "uint256";
        readonly name: "newMax";
    }];
}>;
//# sourceMappingURL=SetMaxKeysPerFid.d.ts.map