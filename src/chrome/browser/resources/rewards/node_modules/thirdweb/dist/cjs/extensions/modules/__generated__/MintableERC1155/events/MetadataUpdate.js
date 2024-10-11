"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.metadataUpdateEvent = metadataUpdateEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the MetadataUpdate event.
 * @returns The prepared event object.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { MintableERC1155 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  MintableERC1155.metadataUpdateEvent()
 * ],
 * });
 * ```
 */
function metadataUpdateEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event MetadataUpdate(uint256 id)",
    });
}
//# sourceMappingURL=MetadataUpdate.js.map