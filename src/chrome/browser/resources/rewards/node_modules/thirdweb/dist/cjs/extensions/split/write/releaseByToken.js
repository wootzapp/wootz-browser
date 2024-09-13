"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.releaseByToken = releaseByToken;
const prepare_contract_call_js_1 = require("../../../transaction/prepare-contract-call.js");
/**
 * Similar to the `release` extension, however this one requires you to specify a tokenAddress to release
 * @param options
 * @returns
 * @example
 * ```ts
 * import { releaseByToken } from "thirdweb/extensions/split";
 *
 * const transaction = releaseByToken({
 *  contract,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * ...
 *
 * ```
 * @extension SPLIT
 */
function releaseByToken(options) {
    return (0, prepare_contract_call_js_1.prepareContractCall)({
        method: "function release(address token, address account)",
        params: [options.tokenAddress, options.account],
        ...options,
    });
}
//# sourceMappingURL=releaseByToken.js.map