"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FN_SELECTOR = void 0;
exports.isUnlockStakeSupported = isUnlockStakeSupported;
exports.unlockStake = unlockStake;
const prepare_contract_call_js_1 = require("../../../../../transaction/prepare-contract-call.js");
const detectExtension_js_1 = require("../../../../../utils/bytecode/detectExtension.js");
exports.FN_SELECTOR = "0xbb9fe6bf";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `unlockStake` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `unlockStake` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isUnlockStakeSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isUnlockStakeSupported(["0x..."]);
 * ```
 */
function isUnlockStakeSupported(availableSelectors) {
    return (0, detectExtension_js_1.detectMethod)({
        availableSelectors,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "unlockStake" function on the contract.
 * @param options - The options for the "unlockStake" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { unlockStake } from "thirdweb/extensions/erc4337";
 *
 * const transaction = unlockStake();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
function unlockStake(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        contract: options.contract,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=unlockStake.js.map