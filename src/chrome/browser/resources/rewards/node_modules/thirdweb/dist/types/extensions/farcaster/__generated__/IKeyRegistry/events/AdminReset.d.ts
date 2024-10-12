import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AdminReset" event.
 */
export type AdminResetEventFilters = Partial<{
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
 * Creates an event object for the AdminReset event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { adminResetEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  adminResetEvent({
 *  fid: ...,
 *  key: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function adminResetEvent(filters?: AdminResetEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AdminReset";
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
//# sourceMappingURL=AdminReset.d.ts.map