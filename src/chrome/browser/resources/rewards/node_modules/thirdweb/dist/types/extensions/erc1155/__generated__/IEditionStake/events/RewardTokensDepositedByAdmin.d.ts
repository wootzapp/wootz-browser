/**
 * Creates an event object for the RewardTokensDepositedByAdmin event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rewardTokensDepositedByAdminEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rewardTokensDepositedByAdminEvent()
 * ],
 * });
 * ```
 */
export declare function rewardTokensDepositedByAdminEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RewardTokensDepositedByAdmin";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_amount";
    }];
}>;
//# sourceMappingURL=RewardTokensDepositedByAdmin.d.ts.map