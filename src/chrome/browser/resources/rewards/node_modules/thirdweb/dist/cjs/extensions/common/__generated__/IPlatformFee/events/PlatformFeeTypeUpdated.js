"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.platformFeeTypeUpdatedEvent = platformFeeTypeUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the PlatformFeeTypeUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { platformFeeTypeUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  platformFeeTypeUpdatedEvent()
 * ],
 * });
 * ```
 */
function platformFeeTypeUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event PlatformFeeTypeUpdated(uint8 feeType)",
    });
}
//# sourceMappingURL=PlatformFeeTypeUpdated.js.map