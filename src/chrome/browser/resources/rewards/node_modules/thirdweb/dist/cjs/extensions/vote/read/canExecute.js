"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.canExecute = canExecute;
const simulate_js_1 = require("../../../transaction/actions/simulate.js");
const executeProposal_js_1 = require("../write/executeProposal.js");
/**
 * Simulate the `execute` method of the Vote contract, to check if you can execute a proposal
 * @extension VOTE
 * @returns boolean - `true` if the proposal is executable, else `false`
 *
 * @example
 * ```ts
 * import { canExecute } from "thirdweb/extensions/vote";
 *
 * const executable = await canExecute({ contract, proposalId });
 * ```
 */
async function canExecute(options) {
    try {
        const transaction = await (0, executeProposal_js_1.executeProposal)(options);
        await (0, simulate_js_1.simulateTransaction)({ transaction });
        return true;
    }
    catch {
        return false;
    }
}
//# sourceMappingURL=canExecute.js.map