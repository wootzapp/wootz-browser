"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.mintableTokenURIUpdatedEvent = mintableTokenURIUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the MintableTokenURIUpdated event.
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
 *  MintableERC1155.mintableTokenURIUpdatedEvent()
 * ],
 * });
 * ```
 */
function mintableTokenURIUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event MintableTokenURIUpdated(uint256 tokenId, string tokenURI)",
    });
}
//# sourceMappingURL=MintableTokenURIUpdated.js.map