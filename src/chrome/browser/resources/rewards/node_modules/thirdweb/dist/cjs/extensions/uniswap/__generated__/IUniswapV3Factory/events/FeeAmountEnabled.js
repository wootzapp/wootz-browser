"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.feeAmountEnabledEvent = feeAmountEnabledEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function feeAmountEnabledEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event FeeAmountEnabled(uint24 fee, int24 tickSpacing)",
    });
}
//# sourceMappingURL=FeeAmountEnabled.js.map