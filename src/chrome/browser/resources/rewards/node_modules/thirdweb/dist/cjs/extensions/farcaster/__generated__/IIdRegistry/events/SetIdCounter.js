"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setIdCounterEvent = setIdCounterEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetIdCounter event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdCounterEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdCounterEvent()
 * ],
 * });
 * ```
 */
function setIdCounterEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetIdCounter(uint256 oldCounter, uint256 newCounter)",
    });
}
//# sourceMappingURL=SetIdCounter.js.map