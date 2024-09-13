"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FN_SELECTOR = void 0;
exports.isDistributeSupported = isDistributeSupported;
exports.distribute = distribute;
const prepare_contract_call_js_1 = require("../../../../../transaction/prepare-contract-call.js");
const detectExtension_js_1 = require("../../../../../utils/bytecode/detectExtension.js");
exports.FN_SELECTOR = "0xe4fc6b6d";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `distribute` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `distribute` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isDistributeSupported } from "thirdweb/extensions/split";
 *
 * const supported = isDistributeSupported(["0x..."]);
 * ```
 */
function isDistributeSupported(availableSelectors) {
    return (0, detectExtension_js_1.detectMethod)({
        availableSelectors,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "distribute" function on the contract.
 * @param options - The options for the "distribute" function.
 * @returns A prepared transaction object.
 * @extension SPLIT
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { distribute } from "thirdweb/extensions/split";
 *
 * const transaction = distribute();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
function distribute(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        contract: options.contract,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=distribute.js.map