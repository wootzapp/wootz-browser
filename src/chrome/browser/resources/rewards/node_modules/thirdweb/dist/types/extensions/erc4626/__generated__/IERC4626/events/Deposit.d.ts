import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Deposit" event.
 */
export type DepositEventFilters = Partial<{
    caller: AbiParameterToPrimitiveType<{
        type: "address";
        name: "caller";
        indexed: true;
    }>;
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Deposit event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4626
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { depositEvent } from "thirdweb/extensions/erc4626";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  depositEvent({
 *  caller: ...,
 *  owner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function depositEvent(filters?: DepositEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Deposit";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "caller";
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
//# sourceMappingURL=Deposit.d.ts.map