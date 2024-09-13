"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.extractIPFSUri = extractIPFSUri;
const encode_js_1 = require("../base58/encode.js");
const to_bytes_js_1 = require("../encoding/to-bytes.js");
const cbor_decode_js_1 = require("./cbor-decode.js");
const prefix_js_1 = require("./prefix.js");
/**
 * Extracts the IPFS URI from the given bytecode.
 * @param bytecode - The bytecode to extract the IPFS URI from.
 * @returns The IPFS URI if found, otherwise undefined.
 * @example
 * ```ts
 * import { extractIPFSUri } from "thirdweb/utils/bytecode/extractIPFS";
 * const bytecode = "0x363d3d373d3d3d363d30545af43d82803e903d91601857fd5bf3";
 * const ipfsHash = extractIPFSUri(bytecode);
 * console.log(ipfsHash);
 * ```
 * @utils
 */
function extractIPFSUri(bytecode) {
    const numericBytecode = (0, to_bytes_js_1.hexToBytes)((0, prefix_js_1.ensureBytecodePrefix)(bytecode));
    const cborLength = 
    // @ts-expect-error - TS doesn't like this, but it's fine
    numericBytecode[numericBytecode.length - 2] * 0x100 +
        // @ts-expect-error - TS doesn't like this, but it's fine
        numericBytecode[numericBytecode.length - 1];
    const cborStart = numericBytecode.length - 2 - cborLength;
    // if the cborStart is invalid, return undefined
    if (cborStart < 0 || cborStart > numericBytecode.length) {
        return undefined;
    }
    const bytecodeBuffer = numericBytecode.slice(cborStart, -2);
    const cborData = (0, cbor_decode_js_1.decode)(bytecodeBuffer);
    if ("ipfs" in cborData) {
        return `ipfs://${(0, encode_js_1.base58Encode)(cborData.ipfs)}`;
    }
    return undefined;
}
//# sourceMappingURL=extractIPFS.js.map