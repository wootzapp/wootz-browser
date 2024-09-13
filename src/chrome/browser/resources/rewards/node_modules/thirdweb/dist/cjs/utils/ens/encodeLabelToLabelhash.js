"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.encodedLabelToLabelhash = encodedLabelToLabelhash;
const hex_js_1 = require("../encoding/hex.js");
/**
 * @internal
 */
function encodedLabelToLabelhash(label) {
    if (label.length !== 66) {
        return null;
    }
    if (label.indexOf("[") !== 0) {
        return null;
    }
    if (label.indexOf("]") !== 65) {
        return null;
    }
    const hash = `0x${label.slice(1, 65)}`;
    if (!(0, hex_js_1.isHex)(hash)) {
        return null;
    }
    return hash;
}
//# sourceMappingURL=encodeLabelToLabelhash.js.map