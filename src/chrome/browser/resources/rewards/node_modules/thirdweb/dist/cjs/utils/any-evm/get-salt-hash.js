"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getSaltHash = getSaltHash;
const prefix_js_1 = require("../bytecode/prefix.js");
const keccak_id_js_1 = require("./keccak-id.js");
/**
 * Calculates the salt hash for a given bytecode.
 * @param bytecode - The bytecode to calculate the salt hash for.
 * @returns The salt hash of the bytecode.
 * @example
 * ```ts
 * import { getSaltHash } from "thirdweb";
 * const saltHash = getSaltHash(bytecode);
 * ```
 * @utils
 */
function getSaltHash(bytecode) {
    const bytecodeHash = (0, keccak_id_js_1.keccakId)((0, prefix_js_1.ensureBytecodePrefix)(bytecode));
    const salt = `tw.${bytecodeHash}`;
    return (0, keccak_id_js_1.keccakId)(salt);
}
//# sourceMappingURL=get-salt-hash.js.map