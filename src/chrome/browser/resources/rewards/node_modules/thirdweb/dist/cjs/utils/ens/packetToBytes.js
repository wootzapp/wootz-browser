"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.packetToBytes = packetToBytes;
const viem_1 = require("viem");
const to_bytes_js_1 = require("../encoding/to-bytes.js");
const encodeLabelhash_js_1 = require("./encodeLabelhash.js");
/**
 * Encodes a DNS packet into a ByteArray containing a UDP payload.
 * @param packet
 * @internal
 */
function packetToBytes(packet) {
    // strip leading and trailing `.`
    const value = packet.replace(/^\.|\.$/gm, "");
    if (value.length === 0) {
        return new Uint8Array(1);
    }
    const bytes = new Uint8Array((0, to_bytes_js_1.stringToBytes)(value).byteLength + 2);
    let offset = 0;
    const list = value.split(".");
    for (let i = 0; i < list.length; i++) {
        const item = list[i];
        let encoded = (0, to_bytes_js_1.stringToBytes)(item);
        // if the length is > 255, make the encoded label value a labelhash
        // this is compatible with the universal resolver
        if (encoded.byteLength > 255) {
            encoded = (0, to_bytes_js_1.stringToBytes)((0, encodeLabelhash_js_1.encodeLabelhash)((0, viem_1.labelhash)(item)));
        }
        bytes[offset] = encoded.length;
        bytes.set(encoded, offset + 1);
        offset += encoded.length + 1;
    }
    if (bytes.byteLength !== offset + 1) {
        return bytes.slice(0, offset + 1);
    }
    return bytes;
}
//# sourceMappingURL=packetToBytes.js.map