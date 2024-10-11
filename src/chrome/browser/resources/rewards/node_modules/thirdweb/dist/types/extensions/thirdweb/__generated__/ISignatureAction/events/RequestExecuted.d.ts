import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RequestExecuted" event.
 */
export type RequestExecutedEventFilters = Partial<{
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
        indexed: true;
    }>;
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RequestExecuted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { requestExecutedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  requestExecutedEvent({
 *  user: ...,
 *  signer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function requestExecutedEvent(filters?: RequestExecutedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RequestExecuted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "user";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "signer";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "uint128";
            readonly name: "validityStartTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "validityEndTimestamp";
        }, {
            readonly type: "bytes32";
            readonly name: "uid";
        }, {
            readonly type: "bytes";
            readonly name: "data";
        }];
        readonly name: "_req";
    }];
}>;
//# sourceMappingURL=RequestExecuted.d.ts.map