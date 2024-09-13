"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.randomBytesHex = randomBytesHex;
exports.randomBytesBuffer = randomBytesBuffer;
const hex_js_1 = require("./encoding/hex.js");
/**
 * @internal
 */
function randomBytesHex(length = 32) {
    return (0, hex_js_1.uint8ArrayToHex)(randomBytesBuffer(length));
}
/**
 * @internal
 */
function randomBytesBuffer(length = 32) {
    return globalThis.crypto.getRandomValues(new Uint8Array(length));
}
//# sourceMappingURL=random.js.map