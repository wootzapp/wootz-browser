"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setValidatorEvent = setValidatorEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetValidator event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setValidatorEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setValidatorEvent()
 * ],
 * });
 * ```
 */
function setValidatorEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetValidator(uint32 keyType, uint8 metadataType, address oldValidator, address newValidator)",
    });
}
//# sourceMappingURL=SetValidator.js.map