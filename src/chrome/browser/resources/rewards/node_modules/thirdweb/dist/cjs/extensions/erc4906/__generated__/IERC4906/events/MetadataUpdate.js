"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.metadataUpdateEvent = metadataUpdateEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the MetadataUpdate event.
 * @returns The prepared event object.
 * @extension ERC4906
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { metadataUpdateEvent } from "thirdweb/extensions/erc4906";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  metadataUpdateEvent()
 * ],
 * });
 * ```
 */
function metadataUpdateEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event MetadataUpdate(uint256 _tokenId)",
    });
}
//# sourceMappingURL=MetadataUpdate.js.map