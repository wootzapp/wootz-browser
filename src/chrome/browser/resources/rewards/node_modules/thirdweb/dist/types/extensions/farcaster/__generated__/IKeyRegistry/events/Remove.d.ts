import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Remove" event.
 */
export type RemoveEventFilters = Partial<{
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
        indexed: true;
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Remove event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { removeEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  removeEvent({
 *  fid: ...,
 *  key: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function removeEvent(filters?: RemoveEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Remove";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "fid";
        readonly indexed: true;
    }, {
        readonly type: "bytes";
        readonly name: "key";
        readonly indexed: true;
    }, {
        readonly type: "bytes";
        readonly name: "keyBytes";
    }];
}>;
//# sourceMappingURL=Remove.d.ts.map