"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.batchMetadataUpdateEvent = batchMetadataUpdateEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the BatchMetadataUpdate event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { batchMetadataUpdateEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  batchMetadataUpdateEvent()
 * ],
 * });
 * ```
 */
function batchMetadataUpdateEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event BatchMetadataUpdate(uint256 _fromTokenId, uint256 _toTokenId)",
    });
}
//# sourceMappingURL=BatchMetadataUpdate.js.map