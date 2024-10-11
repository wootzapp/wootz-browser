"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.keccakId = keccakId;
const to_bytes_js_1 = require("../encoding/to-bytes.js");
const keccak256_js_1 = require("../hashing/keccak256.js");
/**
 * Calculates the keccak ID of the given input.
 * @param input - The input value to calculate the keccak ID for.
 * @returns The keccak ID as a Hex string.
 * @example
 * ```ts
 * import { keccackId } from "thirdweb/utils";
 * const keccakId = keccackId(input);
 * ```
 * @utils
 */
function keccakId(input) {
    return (0, keccak256_js_1.keccak256)((0, to_bytes_js_1.stringToBytes)(input));
}
//# sourceMappingURL=keccak-id.js.map