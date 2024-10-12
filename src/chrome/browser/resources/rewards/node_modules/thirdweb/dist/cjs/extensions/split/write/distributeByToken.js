"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.distributeByToken = distributeByToken;
const prepare_contract_call_js_1 = require("../../../transaction/prepare-contract-call.js");
/**
 * This extension is similar to the `distribute` extension,
 * however it require you to specify the token (address) that you want to distribute
 * @param options.tokenAddress - The contract address of the token you want to distribute
 * @returns A prepared transaction object.
 * @extension SPLIT
 * @example
 * ```ts
 * import { distributeByToken } from "thirdweb/extensions/split";
 *
 * const transaction = distributeByToken();
 *
 * // Send the transaction
 * ...
 *
 * ```
 */
function distributeByToken(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        method: "function distribute(address token)",
        params: [options.tokenAddress],
        ...options,
    });
}
//# sourceMappingURL=distributeByToken.js.map