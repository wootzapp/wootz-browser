import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Withdraw" event.
 */
export type WithdrawEventFilters = Partial<{
    caller: AbiParameterToPrimitiveType<{
        type: "address";
        name: "caller";
        indexed: true;
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
        indexed: true;
    }>;
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Withdraw event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4626
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { withdrawEvent } from "thirdweb/extensions/erc4626";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  withdrawEvent({
 *  caller: ...,
 *  receiver: ...,
 *  owner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function withdrawEvent(filters?: WithdrawEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Withdraw";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "caller";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "receiver";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "owner";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "assets";
    }, {
        readonly type: "uint256";
        readonly name: "shares";
    }];
}>;
//# sourceMappingURL=Withdraw.d.ts.map