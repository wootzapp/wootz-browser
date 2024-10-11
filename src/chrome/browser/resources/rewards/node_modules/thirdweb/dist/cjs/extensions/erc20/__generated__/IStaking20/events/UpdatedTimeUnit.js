"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updatedTimeUnitEvent = updatedTimeUnitEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function updatedTimeUnitEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event UpdatedTimeUnit(uint256 oldTimeUnit, uint256 newTimeUnit)",
    });
}
//# sourceMappingURL=UpdatedTimeUnit.js.map