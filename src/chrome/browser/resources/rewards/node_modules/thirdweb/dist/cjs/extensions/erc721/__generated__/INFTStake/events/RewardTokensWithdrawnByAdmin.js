"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.rewardTokensWithdrawnByAdminEvent = rewardTokensWithdrawnByAdminEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the RewardTokensWithdrawnByAdmin event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rewardTokensWithdrawnByAdminEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rewardTokensWithdrawnByAdminEvent()
 * ],
 * });
 * ```
 */
function rewardTokensWithdrawnByAdminEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event RewardTokensWithdrawnByAdmin(uint256 _amount)",
    });
}
//# sourceMappingURL=RewardTokensWithdrawnByAdmin.js.map