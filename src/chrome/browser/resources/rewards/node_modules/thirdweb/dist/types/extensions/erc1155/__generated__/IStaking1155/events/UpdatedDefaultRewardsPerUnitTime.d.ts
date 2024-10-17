/**
 * Creates an event object for the UpdatedDefaultRewardsPerUnitTime event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedDefaultRewardsPerUnitTimeEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedDefaultRewardsPerUnitTimeEvent()
 * ],
 * });
 * ```
 */
export declare function updatedDefaultRewardsPerUnitTimeEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedDefaultRewardsPerUnitTime";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldRewardsPerUnitTime";
    }, {
        readonly type: "uint256";
        readonly name: "newRewardsPerUnitTime";
    }];
}>;
//# sourceMappingURL=UpdatedDefaultRewardsPerUnitTime.d.ts.map