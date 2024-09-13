"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.airdropWithSignatureEvent = airdropWithSignatureEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the AirdropWithSignature event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropWithSignatureEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropWithSignatureEvent()
 * ],
 * });
 * ```
 */
function airdropWithSignatureEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event AirdropWithSignature(address token)",
    });
}
//# sourceMappingURL=AirdropWithSignature.js.map