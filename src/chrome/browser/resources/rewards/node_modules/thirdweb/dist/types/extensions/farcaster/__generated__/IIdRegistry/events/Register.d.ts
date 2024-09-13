import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Register" event.
 */
export type RegisterEventFilters = Partial<{
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
 * Creates an event object for the Register event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { registerEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  registerEvent({
 *  to: ...,
 *  id: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function registerEvent(filters?: RegisterEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Register";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "to";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "id";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "recovery";
    }];
}>;
//# sourceMappingURL=Register.d.ts.map