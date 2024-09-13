/**
 * Creates an event object for the UpdatedMinStakeAmount event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedMinStakeAmountEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedMinStakeAmountEvent()
 * ],
 * });
 * ```
 */
export declare function updatedMinStakeAmountEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedMinStakeAmount";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldAmount";
    }, {
        readonly type: "uint256";
        readonly name: "newAmount";
    }];
}>;
//# sourceMappingURL=UpdatedMinStakeAmount.d.ts.map