"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.contractURIUpdatedEvent = contractURIUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the ContractURIUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { contractURIUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  contractURIUpdatedEvent()
 * ],
 * });
 * ```
 */
function contractURIUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event ContractURIUpdated(string prevURI, string newURI)",
    });
}
//# sourceMappingURL=ContractURIUpdated.js.map