"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.quorumNumeratorByBlockNumber = quorumNumeratorByBlockNumber;
const read_contract_js_1 = require("../../../transaction/read-contract.js");
/**
 * Calls the "quorumDenominator" function on the contract with an extra param called `blockNumber`.
 * This extension is similar to the `quorumDenominator` extension, except that it takes in a bigint (blockNumber)
 * @param options - The options for the quorumDenominator function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { quorumNumeratorByBlockNumber } from "thirdweb/extensions/vote";
 *
 * const result = await quorumNumeratorByBlockNumber({
 *  contract,
 *  blockNumber: 13232234232n,
 * });
 *
 * ```
 */
async function quorumNumeratorByBlockNumber(options) {
    return (0, read_contract_js_1.readContract)({
        contract: options.contract,
        method: "function quorumNumerator(uint256 blockNumber) view returns (uint256)",
        params: [options.blockNumber],
    });
}
//# sourceMappingURL=quorumNumeratorByBlockNumber.js.map