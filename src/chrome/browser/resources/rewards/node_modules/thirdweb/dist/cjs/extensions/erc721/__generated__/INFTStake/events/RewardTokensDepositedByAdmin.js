"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.rewardTokensDepositedByAdminEvent = rewardTokensDepositedByAdminEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the RewardTokensDepositedByAdmin event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rewardTokensDepositedByAdminEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rewardTokensDepositedByAdminEvent()
 * ],
 * });
 * ```
 */
function rewardTokensDepositedByAdminEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event RewardTokensDepositedByAdmin(uint256 _amount)",
    });
}
//# sourceMappingURL=RewardTokensDepositedByAdmin.js.map