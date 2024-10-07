import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Deposited" event.
 */
export type DepositedEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Deposited event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { depositedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  depositedEvent({
 *  account: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function depositedEvent(filters?: DepositedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Deposited";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "totalDeposit";
    }];
}>;
//# sourceMappingURL=Deposited.d.ts.map