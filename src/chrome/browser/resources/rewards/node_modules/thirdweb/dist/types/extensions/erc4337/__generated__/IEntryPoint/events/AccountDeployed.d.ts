import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AccountDeployed" event.
 */
export type AccountDeployedEventFilters = Partial<{
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
 * Creates an event object for the AccountDeployed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { accountDeployedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  accountDeployedEvent({
 *  userOpHash: ...,
 *  sender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function accountDeployedEvent(filters?: AccountDeployedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AccountDeployed";
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
        readonly name: "factory";
    }, {
        readonly type: "address";
        readonly name: "paymaster";
    }];
}>;
//# sourceMappingURL=AccountDeployed.d.ts.map