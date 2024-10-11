"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.namehash = namehash;
const utils_1 = require("viem/utils");
const to_bytes_js_1 = require("../encoding/to-bytes.js");
const keccak256_js_1 = require("../hashing/keccak256.js");
const encodeLabelToLabelhash_js_1 = require("./encodeLabelToLabelhash.js");
/**
 * @internal
 */
function namehash(name) {
    let result = new Uint8Array(32).fill(0);
    if (!name) {
        return (0, utils_1.bytesToHex)(result);
    }
    const labels = name.split(".");
    // Iterate in reverse order building up hash
    for (let i = labels.length - 1; i >= 0; i -= 1) {
        const item = labels[i];
        const hashFromEncodedLabel = (0, encodeLabelToLabelhash_js_1.encodedLabelToLabelhash)(item);
        const hashed = hashFromEncodedLabel
            ? (0, to_bytes_js_1.toBytes)(hashFromEncodedLabel)
            : (0, keccak256_js_1.keccak256)((0, to_bytes_js_1.stringToBytes)(item), "bytes");
        result = (0, keccak256_js_1.keccak256)((0, utils_1.concat)([result, hashed]), "bytes");
    }
    return (0, utils_1.bytesToHex)(result);
}
//# sourceMappingURL=namehash.js.map