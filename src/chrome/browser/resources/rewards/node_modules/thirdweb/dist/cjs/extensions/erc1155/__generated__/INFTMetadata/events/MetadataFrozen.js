"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.metadataFrozenEvent = metadataFrozenEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the MetadataFrozen event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { metadataFrozenEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  metadataFrozenEvent()
 * ],
 * });
 * ```
 */
function metadataFrozenEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event MetadataFrozen()",
    });
}
//# sourceMappingURL=MetadataFrozen.js.map