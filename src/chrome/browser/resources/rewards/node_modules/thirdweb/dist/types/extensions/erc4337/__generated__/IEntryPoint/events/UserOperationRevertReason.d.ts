import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "UserOperationRevertReason" event.
 */
export type UserOperationRevertReasonEventFilters = Partial<{
    userOpHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "userOpHash";
        indexed: true;
    }>;
    sender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "sender";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the UserOperationRevertReason event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { userOperationRevertReasonEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  userOperationRevertReasonEvent({
 *  userOpHash: ...,
 *  sender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function userOperationRevertReasonEvent(filters?: UserOperationRevertReasonEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UserOperationRevertReason";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "userOpHash";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "sender";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "nonce";
    }, {
        readonly type: "bytes";
        readonly name: "revertReason";
    }];
}>;
//# sourceMappingURL=UserOperationRevertReason.d.ts.map