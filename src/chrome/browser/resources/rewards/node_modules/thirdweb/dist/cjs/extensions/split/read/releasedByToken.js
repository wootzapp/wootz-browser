"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.releasedByToken = releasedByToken;
const read_contract_js_1 = require("../../../transaction/read-contract.js");
/**
 * Calls the "released" function on the contract.
 * Similar to the `released` extension, however this one requires you to specify a tokenAddress
 *
 * @param options - The options for the released function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { releasedByToken } from "thirdweb/extensions/split";
 *
 * const result = await releasedByToken({
 *  contract,
 *  account: "0x...",
 *  tokenAddress: "0x...",
 * });
 *
 * ```
 */
async function releasedByToken(options) {
    return (0, read_contract_js_1.readContract)({
        contract: options.contract,
        method: "function released(address token, address account) view returns (uint256)",
        params: [options.tokenAddress, options.account],
    });
}
//# sourceMappingURL=releasedByToken.js.map