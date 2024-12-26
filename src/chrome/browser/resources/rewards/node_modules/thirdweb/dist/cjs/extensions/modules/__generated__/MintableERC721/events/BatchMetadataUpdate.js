"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.batchMetadataUpdateEvent = batchMetadataUpdateEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the BatchMetadataUpdate event.
 * @returns The prepared event object.
 * @modules MintableERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { MintableERC721 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  MintableERC721.batchMetadataUpdateEvent()
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