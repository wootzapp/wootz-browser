/**
 * Creates an event object for the SetIdCounter event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdCounterEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdCounterEvent()
 * ],
 * });
 * ```
 */
export declare function setIdCounterEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetIdCounter";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldCounter";
    }, {
        readonly type: "uint256";
        readonly name: "newCounter";
    }];
}>;
//# sourceMappingURL=SetIdCounter.d.ts.map