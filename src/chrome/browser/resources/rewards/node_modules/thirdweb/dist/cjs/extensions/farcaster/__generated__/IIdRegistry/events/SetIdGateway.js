"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setIdGatewayEvent = setIdGatewayEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetIdGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdGatewayEvent()
 * ],
 * });
 * ```
 */
function setIdGatewayEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetIdGateway(address oldIdGateway, address newIdGateway)",
    });
}
//# sourceMappingURL=SetIdGateway.js.map