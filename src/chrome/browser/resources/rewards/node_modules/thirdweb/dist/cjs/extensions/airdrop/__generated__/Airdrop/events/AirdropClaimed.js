"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.airdropClaimedEvent = airdropClaimedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the AirdropClaimed event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropClaimedEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropClaimedEvent()
 * ],
 * });
 * ```
 */
function airdropClaimedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event AirdropClaimed(address token, address receiver)",
    });
}
//# sourceMappingURL=AirdropClaimed.js.map