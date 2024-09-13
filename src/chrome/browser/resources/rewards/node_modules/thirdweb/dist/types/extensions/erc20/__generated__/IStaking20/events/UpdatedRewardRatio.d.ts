/**
 * Creates an event object for the UpdatedRewardRatio event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedRewardRatioEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedRewardRatioEvent()
 * ],
 * });
 * ```
 */
export declare function updatedRewardRatioEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedRewardRatio";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldNumerator";
    }, {
        readonly type: "uint256";
        readonly name: "newNumerator";
    }, {
        readonly type: "uint256";
        readonly name: "oldDenominator";
    }, {
        readonly type: "uint256";
        readonly name: "newDenominator";
    }];
}>;
//# sourceMappingURL=UpdatedRewardRatio.d.ts.map