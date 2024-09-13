/**
 * Creates an event object for the Paused event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { pausedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  pausedEvent()
 * ],
 * });
 * ```
 */
export declare function pausedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Paused";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bool";
        readonly name: "isPaused";
    }];
}>;
//# sourceMappingURL=Paused.d.ts.map