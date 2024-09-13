"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.airdropEvent = airdropEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the Airdrop event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropEvent()
 * ],
 * });
 * ```
 */
function airdropEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event Airdrop(address token)",
    });
}
//# sourceMappingURL=Airdrop.js.map