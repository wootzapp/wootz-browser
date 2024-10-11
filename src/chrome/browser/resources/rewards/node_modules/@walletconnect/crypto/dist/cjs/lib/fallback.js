"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.fallbackRipemd160 = exports.fallbackSha512 = exports.fallbackSha256 = exports.fallbackHmacSha512Sign = exports.fallbackHmacSha256Sign = exports.fallbackAesDecrypt = exports.fallbackAesEncrypt = void 0;
const tslib_1 = require("tslib");
const aes_js_1 = tslib_1.__importDefault(require("aes-js"));
const encoding_1 = require("@walletconnect/encoding");
const hash = tslib_1.__importStar(require("hash.js"));
const constants_1 = require("../constants");
const helpers_1 = require("../helpers");
function fallbackAesEncrypt(iv, key, data) {
    const aesCbc = new aes_js_1.default.ModeOfOperation.cbc(key, iv);
    const padded = helpers_1.pkcs7.pad(data);
    const encryptedBytes = aesCbc.encrypt(padded);
    return new Uint8Array(encryptedBytes);
}
exports.fallbackAesEncrypt = fallbackAesEncrypt;
function fallbackAesDecrypt(iv, key, data) {
    const aesCbc = new aes_js_1.default.ModeOfOperation.cbc(key, iv);
    const encryptedBytes = aesCbc.decrypt(data);
    const padded = new Uint8Array(encryptedBytes);
    const result = helpers_1.pkcs7.unpad(padded);
    return result;
}
exports.fallbackAesDecrypt = fallbackAesDecrypt;
function fallbackHmacSha256Sign(key, data) {
    const result = hash
        .hmac(hash[constants_1.SHA256_NODE_ALGO], key)
        .update(data)
        .digest(constants_1.HEX_ENC);
    return encoding_1.hexToArray(result);
}
exports.fallbackHmacSha256Sign = fallbackHmacSha256Sign;
function fallbackHmacSha512Sign(key, data) {
    const result = hash
        .hmac(hash[constants_1.SHA512_NODE_ALGO], key)
        .update(data)
        .digest(constants_1.HEX_ENC);
    return encoding_1.hexToArray(result);
}
exports.fallbackHmacSha512Sign = fallbackHmacSha512Sign;
function fallbackSha256(msg) {
    const result = hash
        .sha256()
        .update(msg)
        .digest(constants_1.HEX_ENC);
    return encoding_1.hexToArray(result);
}
exports.fallbackSha256 = fallbackSha256;
function fallbackSha512(msg) {
    const result = hash
        .sha512()
        .update(msg)
        .digest(constants_1.HEX_ENC);
    return encoding_1.hexToArray(result);
}
exports.fallbackSha512 = fallbackSha512;
function fallbackRipemd160(msg) {
    const result = hash
        .ripemd160()
        .update(msg)
        .digest(constants_1.HEX_ENC);
    return encoding_1.hexToArray(result);
}
exports.fallbackRipemd160 = fallbackRipemd160;
//# sourceMappingURL=fallback.js.map