"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.keccak256 = keccak256;
const sha3_1 = require("@noble/hashes/sha3");
const hex_js_1 = require("../encoding/hex.js");
/**
 * Calculates the Keccak-256 hash of the given value.
 * @param value - The value to hash, either as a hexadecimal string or a Uint8Array.
 * @param to - The desired output format of the hash (optional). Defaults to 'hex'.
 * @returns The Keccak-256 hash of the value in the specified format.
 * @example
 * ```ts
 * import { keccak256 } from "thirdweb/utils";
 * const hash = keccak256("0x1234");
 * ```
 * @utils
 */
function keccak256(value, to) {
    const bytes = (0, sha3_1.keccak_256)((0, hex_js_1.isHex)(value, { strict: false }) ? (0, hex_js_1.hexToUint8Array)(value) : value);
    if (to === "bytes") {
        return bytes;
    }
    // default fall through to hex
    return (0, hex_js_1.uint8ArrayToHex)(bytes);
}
//# sourceMappingURL=keccak256.js.map