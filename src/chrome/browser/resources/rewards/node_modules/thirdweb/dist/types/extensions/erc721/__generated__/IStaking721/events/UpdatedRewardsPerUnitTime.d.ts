/**
 * Creates an event object for the UpdatedRewardsPerUnitTime event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedRewardsPerUnitTimeEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedRewardsPerUnitTimeEvent()
 * ],
 * });
 * ```
 */
export declare function updatedRewardsPerUnitTimeEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedRewardsPerUnitTime";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldRewardsPerUnitTime";
    }, {
        readonly type: "uint256";
        readonly name: "newRewardsPerUnitTime";
    }];
}>;
//# sourceMappingURL=UpdatedRewardsPerUnitTime.d.ts.map