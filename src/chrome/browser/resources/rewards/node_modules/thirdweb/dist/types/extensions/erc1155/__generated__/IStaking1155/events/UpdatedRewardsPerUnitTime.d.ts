import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "UpdatedRewardsPerUnitTime" event.
 */
export type UpdatedRewardsPerUnitTimeEventFilters = Partial<{
    _tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the UpdatedRewardsPerUnitTime event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedRewardsPerUnitTimeEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedRewardsPerUnitTimeEvent({
 *  _tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function updatedRewardsPerUnitTimeEvent(filters?: UpdatedRewardsPerUnitTimeEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedRewardsPerUnitTime";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_tokenId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "oldRewardsPerUnitTime";
    }, {
        readonly type: "uint256";
        readonly name: "newRewardsPerUnitTime";
    }];
}>;
//# sourceMappingURL=UpdatedRewardsPerUnitTime.d.ts.map