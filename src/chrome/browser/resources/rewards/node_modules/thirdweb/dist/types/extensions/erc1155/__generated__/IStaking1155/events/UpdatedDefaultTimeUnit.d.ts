/**
 * Creates an event object for the UpdatedDefaultTimeUnit event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedDefaultTimeUnitEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedDefaultTimeUnitEvent()
 * ],
 * });
 * ```
 */
export declare function updatedDefaultTimeUnitEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedDefaultTimeUnit";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldTimeUnit";
    }, {
        readonly type: "uint256";
        readonly name: "newTimeUnit";
    }];
}>;
//# sourceMappingURL=UpdatedDefaultTimeUnit.d.ts.map