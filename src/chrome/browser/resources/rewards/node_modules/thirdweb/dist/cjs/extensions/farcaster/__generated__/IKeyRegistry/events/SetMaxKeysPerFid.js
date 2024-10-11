"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setMaxKeysPerFidEvent = setMaxKeysPerFidEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetMaxKeysPerFid event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setMaxKeysPerFidEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setMaxKeysPerFidEvent()
 * ],
 * });
 * ```
 */
function setMaxKeysPerFidEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetMaxKeysPerFid(uint256 oldMax, uint256 newMax)",
    });
}
//# sourceMappingURL=SetMaxKeysPerFid.js.map