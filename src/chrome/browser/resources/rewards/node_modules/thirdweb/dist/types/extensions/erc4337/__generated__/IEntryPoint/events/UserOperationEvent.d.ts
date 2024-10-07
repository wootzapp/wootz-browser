import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "UserOperationEvent" event.
 */
export type UserOperationEventEventFilters = Partial<{
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
    paymaster: AbiParameterToPrimitiveType<{
        type: "address";
        name: "paymaster";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the UserOperationEvent event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { userOperationEventEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  userOperationEventEvent({
 *  userOpHash: ...,
 *  sender: ...,
 *  paymaster: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function userOperationEventEvent(filters?: UserOperationEventEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UserOperationEvent";
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
        readonly type: "address";
        readonly name: "paymaster";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "nonce";
    }, {
        readonly type: "bool";
        readonly name: "success";
    }, {
        readonly type: "uint256";
        readonly name: "actualGasCost";
    }, {
        readonly type: "uint256";
        readonly name: "actualGasUsed";
    }];
}>;
//# sourceMappingURL=UserOperationEvent.d.ts.map