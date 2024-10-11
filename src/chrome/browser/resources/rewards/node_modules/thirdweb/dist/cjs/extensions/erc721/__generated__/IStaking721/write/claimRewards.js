"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FN_SELECTOR = void 0;
exports.isClaimRewardsSupported = isClaimRewardsSupported;
exports.claimRewards = claimRewards;
const prepare_contract_call_js_1 = require("../../../../../transaction/prepare-contract-call.js");
const detectExtension_js_1 = require("../../../../../utils/bytecode/detectExtension.js");
exports.FN_SELECTOR = "0x372500ab";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `claimRewards` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimRewards` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isClaimRewardsSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isClaimRewardsSupported(["0x..."]);
 * ```
 */
function isClaimRewardsSupported(availableSelectors) {
    return (0, detectExtension_js_1.detectMethod)({
        availableSelectors,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "claimRewards" function on the contract.
 * @param options - The options for the "claimRewards" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimRewards } from "thirdweb/extensions/erc721";
 *
 * const transaction = claimRewards();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
function claimRewards(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        contract: options.contract,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=claimRewards.js.map