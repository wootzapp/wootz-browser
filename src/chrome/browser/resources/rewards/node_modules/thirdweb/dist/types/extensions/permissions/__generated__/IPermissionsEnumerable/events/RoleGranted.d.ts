import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RoleGranted" event.
 */
export type RoleGrantedEventFilters = Partial<{
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
        indexed: true;
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
    sender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "sender";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RoleGranted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { roleGrantedEvent } from "thirdweb/extensions/permissions";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  roleGrantedEvent({
 *  role: ...,
 *  account: ...,
 *  sender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function roleGrantedEvent(filters?: RoleGrantedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RoleGranted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "role";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "sender";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RoleGranted.d.ts.map