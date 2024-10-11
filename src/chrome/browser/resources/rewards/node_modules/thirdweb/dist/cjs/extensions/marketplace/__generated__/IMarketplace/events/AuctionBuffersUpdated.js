"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.auctionBuffersUpdatedEvent = auctionBuffersUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the AuctionBuffersUpdated event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { auctionBuffersUpdatedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  auctionBuffersUpdatedEvent()
 * ],
 * });
 * ```
 */
function auctionBuffersUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event AuctionBuffersUpdated(uint256 timeBuffer, uint256 bidBufferBps)",
    });
}
//# sourceMappingURL=AuctionBuffersUpdated.js.map