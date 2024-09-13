"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updatedRewardRatioEvent = updatedRewardRatioEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function updatedRewardRatioEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event UpdatedRewardRatio(uint256 oldNumerator, uint256 newNumerator, uint256 oldDenominator, uint256 newDenominator)",
    });
}
//# sourceMappingURL=UpdatedRewardRatio.js.map