"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.freezeIdGatewayEvent = freezeIdGatewayEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the FreezeIdGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { freezeIdGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  freezeIdGatewayEvent()
 * ],
 * });
 * ```
 */
function freezeIdGatewayEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event FreezeIdGateway(address idGateway)",
    });
}
//# sourceMappingURL=FreezeIdGateway.js.map