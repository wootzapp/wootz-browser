"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.sha256 = sha256;
const sha256_1 = require("@noble/hashes/sha256");
const hex_js_1 = require("../encoding/hex.js");
/**
 * Calculates the SHA256 hash of the given value.
 * @param value - The value to hash. It can be either a hexadecimal string or a Uint8Array.
 * @param to - (Optional) The desired output format of the hash. Defaults to 'hex'.
 * @returns The SHA256 hash of the value in the specified format.
 * @example
 * ```ts
 * import { sha256 } from "thirdweb/utils";
 * const hash = sha256("0x1234");
 * ```
 * @utils
 */
function sha256(value, to) {
    const bytes = (0, sha256_1.sha256)((0, hex_js_1.isHex)(value, { strict: false }) ? (0, hex_js_1.hexToUint8Array)(value) : value);
    if (to === "bytes") {
        return bytes;
    }
    return (0, hex_js_1.uint8ArrayToHex)(bytes);
}
//# sourceMappingURL=sha256.js.map