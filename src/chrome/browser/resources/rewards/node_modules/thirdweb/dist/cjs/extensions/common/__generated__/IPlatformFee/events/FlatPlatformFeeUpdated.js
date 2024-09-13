"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.flatPlatformFeeUpdatedEvent = flatPlatformFeeUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the FlatPlatformFeeUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { flatPlatformFeeUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  flatPlatformFeeUpdatedEvent()
 * ],
 * });
 * ```
 */
function flatPlatformFeeUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event FlatPlatformFeeUpdated(address platformFeeRecipient, uint256 flatFee)",
    });
}
//# sourceMappingURL=FlatPlatformFeeUpdated.js.map