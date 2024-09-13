import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "StakeUnlocked" event.
 */
export type StakeUnlockedEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the StakeUnlocked event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { stakeUnlockedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  stakeUnlockedEvent({
 *  account: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function stakeUnlockedEvent(filters?: StakeUnlockedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "StakeUnlocked";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "withdrawTime";
    }];
}>;
//# sourceMappingURL=StakeUnlocked.d.ts.map