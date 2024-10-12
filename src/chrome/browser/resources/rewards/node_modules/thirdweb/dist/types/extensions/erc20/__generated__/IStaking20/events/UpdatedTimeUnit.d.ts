/**
 * Creates an event object for the UpdatedTimeUnit event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedTimeUnitEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedTimeUnitEvent()
 * ],
 * });
 * ```
 */
export declare function updatedTimeUnitEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "UpdatedTimeUnit";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "oldTimeUnit";
    }, {
        readonly type: "uint256";
        readonly name: "newTimeUnit";
    }];
}>;
//# sourceMappingURL=UpdatedTimeUnit.d.ts.map