"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setIdRegistryEvent = setIdRegistryEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetIdRegistry event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdRegistryEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdRegistryEvent()
 * ],
 * });
 * ```
 */
function setIdRegistryEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetIdRegistry(address oldIdRegistry, address newIdRegistry)",
    });
}
//# sourceMappingURL=SetIdRegistry.js.map