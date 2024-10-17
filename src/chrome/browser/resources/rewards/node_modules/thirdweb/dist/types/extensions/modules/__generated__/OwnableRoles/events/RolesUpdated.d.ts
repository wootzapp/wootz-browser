import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RolesUpdated" event.
 */
export type RolesUpdatedEventFilters = Partial<{
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
        indexed: true;
    }>;
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RolesUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rolesUpdatedEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rolesUpdatedEvent({
 *  user: ...,
 *  roles: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function rolesUpdatedEvent(filters?: RolesUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RolesUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "user";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "roles";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RolesUpdated.d.ts.map