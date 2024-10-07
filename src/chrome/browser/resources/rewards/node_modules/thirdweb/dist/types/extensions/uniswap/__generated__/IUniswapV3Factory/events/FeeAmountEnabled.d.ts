/**
 * Creates an event object for the FeeAmountEnabled event.
 * @returns The prepared event object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { feeAmountEnabledEvent } from "thirdweb/extensions/uniswap";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  feeAmountEnabledEvent()
 * ],
 * });
 * ```
 */
export declare function feeAmountEnabledEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "FeeAmountEnabled";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint24";
        readonly name: "fee";
    }, {
        readonly type: "int24";
        readonly name: "tickSpacing";
    }];
}>;
//# sourceMappingURL=FeeAmountEnabled.d.ts.map