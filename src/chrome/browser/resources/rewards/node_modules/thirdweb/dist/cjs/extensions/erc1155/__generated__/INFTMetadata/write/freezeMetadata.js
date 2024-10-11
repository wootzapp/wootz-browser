"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.FN_SELECTOR = void 0;
exports.isFreezeMetadataSupported = isFreezeMetadataSupported;
exports.freezeMetadata = freezeMetadata;
const prepare_contract_call_js_1 = require("../../../../../transaction/prepare-contract-call.js");
const detectExtension_js_1 = require("../../../../../utils/bytecode/detectExtension.js");
exports.FN_SELECTOR = "0xd111515d";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `freezeMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `freezeMetadata` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isFreezeMetadataSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isFreezeMetadataSupported(["0x..."]);
 * ```
 */
function isFreezeMetadataSupported(availableSelectors) {
    return (0, detectExtension_js_1.detectMethod)({
        availableSelectors,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "freezeMetadata" function on the contract.
 * @param options - The options for the "freezeMetadata" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { freezeMetadata } from "thirdweb/extensions/erc1155";
 *
 * const transaction = freezeMetadata();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
function freezeMetadata(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        contract: options.contract,
        method: [exports.FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=freezeMetadata.js.map