"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updatedDefaultTimeUnitEvent = updatedDefaultTimeUnitEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function updatedDefaultTimeUnitEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event UpdatedDefaultTimeUnit(uint256 oldTimeUnit, uint256 newTimeUnit)",
    });
}
//# sourceMappingURL=UpdatedDefaultTimeUnit.js.map