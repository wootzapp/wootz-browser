/**
 * Creates an event object for the RewardTokensWithdrawnByAdmin event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rewardTokensWithdrawnByAdminEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rewardTokensWithdrawnByAdminEvent()
 * ],
 * });
 * ```
 */
export declare function rewardTokensWithdrawnByAdminEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RewardTokensWithdrawnByAdmin";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_amount";
    }];
}>;
//# sourceMappingURL=RewardTokensWithdrawnByAdmin.d.ts.map