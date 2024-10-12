import { prepareContractCall } from "../../../../../transaction/prepare-contract-call.js";
import { detectMethod } from "../../../../../utils/bytecode/detectExtension.js";
export const FN_SELECTOR = "0x25692962";
const FN_INPUTS = [];
const FN_OUTPUTS = [];
/**
 * Checks if the `requestOwnershipHandover` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `requestOwnershipHandover` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRequestOwnershipHandoverSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRequestOwnershipHandoverSupported(["0x..."]);
 * ```
 */
export function isRequestOwnershipHandoverSupported(availableSelectors) {
    return detectMethod({
        availableSelectors,
        method: [FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
/**
 * Prepares a transaction to call the "requestOwnershipHandover" function on the contract.
 * @param options - The options for the "requestOwnershipHandover" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { requestOwnershipHandover } from "thirdweb/extensions/modules";
 *
 * const transaction = requestOwnershipHandover();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export function requestOwnershipHandover(options) {
    return prepareContractCall({
        contract: options.contract,
        method: [FN_SELECTOR, FN_INPUTS, FN_OUTPUTS],
    });
}
//# sourceMappingURL=requestOwnershipHandover.js.map