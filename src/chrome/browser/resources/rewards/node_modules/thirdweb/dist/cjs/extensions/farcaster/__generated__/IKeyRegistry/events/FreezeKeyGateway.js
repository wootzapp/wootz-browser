"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.freezeKeyGatewayEvent = freezeKeyGatewayEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the FreezeKeyGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { freezeKeyGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  freezeKeyGatewayEvent()
 * ],
 * });
 * ```
 */
function freezeKeyGatewayEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event FreezeKeyGateway(address keyGateway)",
    });
}
//# sourceMappingURL=FreezeKeyGateway.js.map