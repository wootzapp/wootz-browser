"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.create2Address = create2Address;
const viem_1 = require("viem");
const address_js_1 = require("../../address.js");
const to_bytes_js_1 = require("../../encoding/to-bytes.js");
const keccak256_js_1 = require("../../hashing/keccak256.js");
function create2Address(options) {
    const { sender, bytecodeHash, salt, input } = options;
    const prefix = (0, keccak256_js_1.keccak256)((0, to_bytes_js_1.stringToBytes)("zksyncCreate2"));
    const inputHash = input ? (0, keccak256_js_1.keccak256)(input) : "0x";
    const addressBytes = (0, keccak256_js_1.keccak256)((0, viem_1.concat)([
        prefix,
        (0, viem_1.padHex)(sender, { size: 32 }),
        salt,
        bytecodeHash,
        inputHash,
    ])).slice(26);
    return (0, address_js_1.getAddress)(`0x${addressBytes}`);
}
//# sourceMappingURL=create2Address.js.map