"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updatedDefaultRewardsPerUnitTimeEvent = updatedDefaultRewardsPerUnitTimeEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function updatedDefaultRewardsPerUnitTimeEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event UpdatedDefaultRewardsPerUnitTime(uint256 oldRewardsPerUnitTime, uint256 newRewardsPerUnitTime)",
    });
}
//# sourceMappingURL=UpdatedDefaultRewardsPerUnitTime.js.map