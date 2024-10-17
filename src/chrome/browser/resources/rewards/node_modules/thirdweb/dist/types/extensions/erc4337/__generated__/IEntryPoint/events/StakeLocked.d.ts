import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "StakeLocked" event.
 */
export type StakeLockedEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the StakeLocked event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { stakeLockedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  stakeLockedEvent({
 *  account: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function stakeLockedEvent(filters?: StakeLockedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "StakeLocked";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "totalStaked";
    }, {
        readonly type: "uint256";
        readonly name: "unstakeDelaySec";
    }];
}>;
//# sourceMappingURL=StakeLocked.d.ts.map