"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.setStorageRegistryEvent = setStorageRegistryEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SetStorageRegistry event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setStorageRegistryEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setStorageRegistryEvent()
 * ],
 * });
 * ```
 */
function setStorageRegistryEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SetStorageRegistry(address oldStorageRegistry, address newStorageRegistry)",
    });
}
//# sourceMappingURL=SetStorageRegistry.js.map