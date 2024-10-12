import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Recover" event.
 */
export type RecoverEventFilters = Partial<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
        indexed: true;
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
        indexed: true;
    }>;
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "id";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Recover event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { recoverEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  recoverEvent({
 *  from: ...,
 *  to: ...,
 *  id: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function recoverEvent(filters?: RecoverEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Recover";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "from";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "to";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "id";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=Recover.d.ts.map