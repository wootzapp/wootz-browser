import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RewardsClaimed" event.
 */
export type RewardsClaimedEventFilters = Partial<{
    staker: AbiParameterToPrimitiveType<{
        type: "address";
        name: "staker";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RewardsClaimed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rewardsClaimedEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rewardsClaimedEvent({
 *  staker: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function rewardsClaimedEvent(filters?: RewardsClaimedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RewardsClaimed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "staker";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "rewardAmount";
    }];
}>;
//# sourceMappingURL=RewardsClaimed.d.ts.map