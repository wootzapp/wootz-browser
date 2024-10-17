import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Add" event.
 */
export type AddEventFilters = Partial<{
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
        indexed: true;
    }>;
    keyType: AbiParameterToPrimitiveType<{
        type: "uint32";
        name: "keyType";
        indexed: true;
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Add event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { addEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  addEvent({
 *  fid: ...,
 *  keyType: ...,
 *  key: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function addEvent(filters?: AddEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Add";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "fid";
        readonly indexed: true;
    }, {
        readonly type: "uint32";
        readonly name: "keyType";
        readonly indexed: true;
    }, {
        readonly type: "bytes";
        readonly name: "key";
        readonly indexed: true;
    }, {
        readonly type: "bytes";
        readonly name: "keyBytes";
    }, {
        readonly type: "uint8";
        readonly name: "metadataType";
    }, {
        readonly type: "bytes";
        readonly name: "metadata";
    }];
}>;
//# sourceMappingURL=Add.d.ts.map