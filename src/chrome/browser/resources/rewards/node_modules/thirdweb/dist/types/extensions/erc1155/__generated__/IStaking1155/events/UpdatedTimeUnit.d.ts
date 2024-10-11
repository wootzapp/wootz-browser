import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "UpdatedTimeUnit" event.
 */
export type UpdatedTimeUnitEventFilters = Partial<{
    _tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the UpdatedTimeUnit event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedTimeUnitEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedTimeUnitEvent({
 *  _tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function updatedTimeUnitEvent(filters?: UpdatedTimeUnitEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedTimeUnit";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_tokenId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "oldTimeUnit";
    }, {
        readonly type: "uint256";
        readonly name: "newTimeUnit";
    }];
}>;
//# sourceMappingURL=UpdatedTimeUnit.d.ts.map