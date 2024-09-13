import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "StakeWithdrawn" event.
 */
export type StakeWithdrawnEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the StakeWithdrawn event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { stakeWithdrawnEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  stakeWithdrawnEvent({
 *  account: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function stakeWithdrawnEvent(filters?: StakeWithdrawnEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "StakeWithdrawn";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "withdrawAddress";
    }, {
        readonly type: "uint256";
        readonly name: "amount";
    }];
}>;
//# sourceMappingURL=StakeWithdrawn.d.ts.map