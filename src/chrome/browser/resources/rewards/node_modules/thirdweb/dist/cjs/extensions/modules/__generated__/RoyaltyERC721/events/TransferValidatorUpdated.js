"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.transferValidatorUpdatedEvent = transferValidatorUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the TransferValidatorUpdated event.
 * @returns The prepared event object.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { RoyaltyERC721 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  RoyaltyERC721.transferValidatorUpdatedEvent()
 * ],
 * });
 * ```
 */
function transferValidatorUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event TransferValidatorUpdated(address oldValidator, address newValidator)",
    });
}
//# sourceMappingURL=TransferValidatorUpdated.js.map