"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.pausedEvent = pausedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the Paused event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { pausedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  pausedEvent()
 * ],
 * });
 * ```
 */
function pausedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event Paused(bool isPaused)",
    });
}
//# sourceMappingURL=Paused.js.map