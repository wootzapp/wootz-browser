"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.proposalExists = proposalExists;
const state_js_1 = require("../__generated__/Vote/read/state.js");
/**
 * Check if a proposal exists based on a given proposalId
 * @returns `true` if the proposal exists, else `false`
 * @extension VOTE
 *
 * @example
 * ```ts
 * import { proposalExists } from "thirdweb/extensions/vote";
 *
 * // Check if the proposal with proposalId `4` exists
 * const exists = await proposalExists({ contract, proposalId: 4n }); // either `true` or `false`
 * ```
 */
async function proposalExists(options) {
    try {
        await (0, state_js_1.state)(options);
        return true;
    }
    catch {
        // If it throws an error then the proposal(id) doesn't exist
        return false;
    }
}
//# sourceMappingURL=proposalExists.js.map