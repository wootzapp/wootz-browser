import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AdminUpdated" event.
 */
export type AdminUpdatedEventFilters = Partial<{
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the AdminUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { adminUpdatedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  adminUpdatedEvent({
 *  signer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function adminUpdatedEvent(filters?: AdminUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AdminUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "signer";
        readonly indexed: true;
    }, {
        readonly type: "bool";
        readonly name: "isAdmin";
    }];
}>;
//# sourceMappingURL=AdminUpdated.d.ts.map