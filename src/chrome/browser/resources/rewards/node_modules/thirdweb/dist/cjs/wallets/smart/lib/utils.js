"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.generateRandomUint192 = void 0;
exports.hexlifyUserOp = hexlifyUserOp;
const hex_js_1 = require("../../../utils/encoding/hex.js");
const generateRandomUint192 = () => {
    const rand1 = BigInt(Math.floor(Math.random() * 0x100000000));
    const rand2 = BigInt(Math.floor(Math.random() * 0x100000000));
    const rand3 = BigInt(Math.floor(Math.random() * 0x100000000));
    const rand4 = BigInt(Math.floor(Math.random() * 0x100000000));
    const rand5 = BigInt(Math.floor(Math.random() * 0x100000000));
    const rand6 = BigInt(Math.floor(Math.random() * 0x100000000));
    return ((rand1 << BigInt(160)) |
        (rand2 << BigInt(128)) |
        (rand3 << BigInt(96)) |
        (rand4 << BigInt(64)) |
        (rand5 << BigInt(32)) |
        rand6);
};
exports.generateRandomUint192 = generateRandomUint192;
/**
 * @internal
 */
function hexlifyUserOp(userOp) {
    return Object.fromEntries(Object.entries(userOp).map(([key, val]) => [
        key,
        // turn any value that's not hex into hex
        val === undefined || val === null || (0, hex_js_1.isHex)(val) ? val : (0, hex_js_1.toHex)(val),
    ]));
}
//# sourceMappingURL=utils.js.map