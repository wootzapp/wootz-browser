"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.sharedMetadataUpdatedEvent = sharedMetadataUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the SharedMetadataUpdated event.
 * @returns The prepared event object.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  OpenEditionMetadataERC721.sharedMetadataUpdatedEvent()
 * ],
 * });
 * ```
 */
function sharedMetadataUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event SharedMetadataUpdated(string name, string description, string imageURI, string animationURI)",
    });
}
//# sourceMappingURL=SharedMetadataUpdated.js.map