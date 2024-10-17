"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.claimConditionsUpdatedEvent = claimConditionsUpdatedEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
/**
 * Creates an event object for the ClaimConditionsUpdated event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { claimConditionsUpdatedEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  claimConditionsUpdatedEvent()
 * ],
 * });
 * ```
 */
function claimConditionsUpdatedEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event ClaimConditionsUpdated((uint256 startTimestamp, uint256 maxClaimableSupply, uint256 supplyClaimed, uint256 quantityLimitPerWallet, bytes32 merkleRoot, uint256 pricePerToken, address currency, string metadata)[] claimConditions, bool resetEligibility)",
    });
}
//# sourceMappingURL=ClaimConditionsUpdated.js.map