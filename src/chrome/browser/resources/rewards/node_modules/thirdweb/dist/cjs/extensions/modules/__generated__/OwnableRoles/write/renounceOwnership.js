"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FN_SELECTOR = void 0;
exports.isRenounceOwnershipSupported = isRenounceOwnershipSupported;
exports.renounceOwnership = renounceOwnership;
const prepare_contract_call_js_1 = require("../../../../../transaction/prepare-contract-call.js");
const detectExtension_js_1 = require("../../../../../utils/bytecode/detectExtension.js");
exports.FN_SELECTOR = "0x715018a6";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `renounceOwnership` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `renounceOwnership` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRenounceOwnershipSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRenounceOwnershipSupported(["0x..."]);
 * ```
 */
function isRenounceOwnershipSupported(availableSelectors) {
    return (0, detectExtension_js_1.detectMethod)({
        availableSelectors,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "renounceOwnership" function on the contract.
 * @param options - The options for the "renounceOwnership" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { renounceOwnership } from "thirdweb/extensions/modules";
 *
 * const transaction = renounceOwnership();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
function renounceOwnership(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        contract: options.contract,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=renounceOwnership.js.map