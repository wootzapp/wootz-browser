/**
 * Creates an event object for the AppURIUpdated event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { appURIUpdatedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  appURIUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function appURIUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AppURIUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "string";
        readonly name: "prevURI";
    }, {
        readonly type: "string";
        readonly name: "newURI";
    }];
}>;
//# sourceMappingURL=AppURIUpdated.d.ts.map