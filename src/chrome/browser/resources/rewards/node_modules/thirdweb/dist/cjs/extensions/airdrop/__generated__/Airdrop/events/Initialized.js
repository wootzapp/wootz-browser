"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.initializedEvent = initializedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the Initialized event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { initializedEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  initializedEvent()
 * ],
 * });
 * ```
 */
function initializedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event Initialized(uint8 version)",
    });
}
//# sourceMappingURL=Initialized.js.map