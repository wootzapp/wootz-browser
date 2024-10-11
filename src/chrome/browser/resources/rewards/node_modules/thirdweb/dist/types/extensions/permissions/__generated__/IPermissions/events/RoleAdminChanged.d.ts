import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RoleAdminChanged" event.
 */
export type RoleAdminChangedEventFilters = Partial<{
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
        indexed: true;
    }>;
    previousAdminRole: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "previousAdminRole";
        indexed: true;
    }>;
    newAdminRole: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "newAdminRole";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RoleAdminChanged event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { roleAdminChangedEvent } from "thirdweb/extensions/permissions";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  roleAdminChangedEvent({
 *  role: ...,
 *  previousAdminRole: ...,
 *  newAdminRole: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function roleAdminChangedEvent(filters?: RoleAdminChangedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RoleAdminChanged";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "role";
        readonly indexed: true;
    }, {
        readonly type: "bytes32";
        readonly name: "previousAdminRole";
        readonly indexed: true;
    }, {
        readonly type: "bytes32";
        readonly name: "newAdminRole";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RoleAdminChanged.d.ts.map