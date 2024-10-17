"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updatedMinStakeAmountEvent = updatedMinStakeAmountEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the UpdatedMinStakeAmount event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { updatedMinStakeAmountEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  updatedMinStakeAmountEvent()
 * ],
 * });
 * ```
 */
function updatedMinStakeAmountEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event UpdatedMinStakeAmount(uint256 oldAmount, uint256 newAmount)",
    });
}
//# sourceMappingURL=UpdatedMinStakeAmount.js.map