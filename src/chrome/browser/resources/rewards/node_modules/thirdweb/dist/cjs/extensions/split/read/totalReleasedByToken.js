"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.totalReleasedByToken = totalReleasedByToken;
const read_contract_js_1 = require("../../../transaction/read-contract.js");
/**
 * Calls the "totalReleased" function on the contract.
 * Similar to the `release` extension, however this one requires you to specify a tokenAddress
 *
 * @param options - The options for the totalReleased function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { totalReleasedByToken } from "thirdweb/extensions/split";
 *
 * const result = await totalReleasedByToken({
 *  contract,
 *  tokenAddress: "0x...",
 * });
 *
 * ```
 */
async function totalReleasedByToken(options) {
    return (0, read_contract_js_1.readContract)({
        contract: options.contract,
        method: "function totalReleased(address token) view returns (uint256)",
        params: [options.tokenAddress],
    });
}
//# sourceMappingURL=totalReleasedByToken.js.map