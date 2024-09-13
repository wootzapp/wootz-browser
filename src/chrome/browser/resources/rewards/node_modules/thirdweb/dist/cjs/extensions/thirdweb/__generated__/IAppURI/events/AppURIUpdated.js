"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.appURIUpdatedEvent = appURIUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the AppURIUpdated event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { appURIUpdatedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  appURIUpdatedEvent()
 * ],
 * });
 * ```
 */
function appURIUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event AppURIUpdated(string prevURI, string newURI)",
    });
}
//# sourceMappingURL=AppURIUpdated.js.map