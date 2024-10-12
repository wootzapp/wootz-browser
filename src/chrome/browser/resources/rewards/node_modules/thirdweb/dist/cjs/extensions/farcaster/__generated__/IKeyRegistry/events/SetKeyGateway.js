"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setKeyGatewayEvent = setKeyGatewayEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetKeyGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setKeyGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setKeyGatewayEvent()
 * ],
 * });
 * ```
 */
function setKeyGatewayEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetKeyGateway(address oldKeyGateway, address newKeyGateway)",
    });
}
//# sourceMappingURL=SetKeyGateway.js.map