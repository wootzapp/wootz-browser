"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.decodeJWT = exports.encodeJWT = exports.decodeData = exports.encodeData = exports.decodeSig = exports.encodeSig = exports.decodeIss = exports.encodeIss = exports.encodeJSON = exports.decodeJSON = void 0;
const concat_1 = require("uint8arrays/concat");
const to_string_1 = require("uint8arrays/to-string");
const from_string_1 = require("uint8arrays/from-string");
const safe_json_1 = require("@walletconnect/safe-json");
const constants_1 = require("./constants");
function decodeJSON(str) {
    return safe_json_1.safeJsonParse(to_string_1.toString(from_string_1.fromString(str, constants_1.JWT_ENCODING), constants_1.JSON_ENCODING));
}
exports.decodeJSON = decodeJSON;
function encodeJSON(val) {
    return to_string_1.toString(from_string_1.fromString(safe_json_1.safeJsonStringify(val), constants_1.JSON_ENCODING), constants_1.JWT_ENCODING);
}
exports.encodeJSON = encodeJSON;
function encodeIss(publicKey) {
    const header = from_string_1.fromString(constants_1.MULTICODEC_ED25519_HEADER, constants_1.MULTICODEC_ED25519_ENCODING);
    const multicodec = constants_1.MULTICODEC_ED25519_BASE +
        to_string_1.toString(concat_1.concat([header, publicKey]), constants_1.MULTICODEC_ED25519_ENCODING);
    return [constants_1.DID_PREFIX, constants_1.DID_METHOD, multicodec].join(constants_1.DID_DELIMITER);
}
exports.encodeIss = encodeIss;
function decodeIss(issuer) {
    const [prefix, method, multicodec] = issuer.split(constants_1.DID_DELIMITER);
    if (prefix !== constants_1.DID_PREFIX || method !== constants_1.DID_METHOD) {
        throw new Error(`Issuer must be a DID with method "key"`);
    }
    const base = multicodec.slice(0, 1);
    if (base !== constants_1.MULTICODEC_ED25519_BASE) {
        throw new Error(`Issuer must be a key in mulicodec format`);
    }
    const bytes = from_string_1.fromString(multicodec.slice(1), constants_1.MULTICODEC_ED25519_ENCODING);
    const type = to_string_1.toString(bytes.slice(0, 2), constants_1.MULTICODEC_ED25519_ENCODING);
    if (type !== constants_1.MULTICODEC_ED25519_HEADER) {
        throw new Error(`Issuer must be a public key with type "Ed25519"`);
    }
    const publicKey = bytes.slice(2);
    if (publicKey.length !== constants_1.MULTICODEC_ED25519_LENGTH) {
        throw new Error(`Issuer must be a public key with length 32 bytes`);
    }
    return publicKey;
}
exports.decodeIss = decodeIss;
function encodeSig(bytes) {
    return to_string_1.toString(bytes, constants_1.JWT_ENCODING);
}
exports.encodeSig = encodeSig;
function decodeSig(encoded) {
    return from_string_1.fromString(encoded, constants_1.JWT_ENCODING);
}
exports.decodeSig = decodeSig;
function encodeData(params) {
    return from_string_1.fromString([encodeJSON(params.header), encodeJSON(params.payload)].join(constants_1.JWT_DELIMITER), constants_1.DATA_ENCODING);
}
exports.encodeData = encodeData;
function decodeData(data) {
    const params = to_string_1.toString(data, constants_1.DATA_ENCODING).split(constants_1.JWT_DELIMITER);
    const header = decodeJSON(params[0]);
    const payload = decodeJSON(params[1]);
    return { header, payload };
}
exports.decodeData = decodeData;
function encodeJWT(params) {
    return [
        encodeJSON(params.header),
        encodeJSON(params.payload),
        encodeSig(params.signature),
    ].join(constants_1.JWT_DELIMITER);
}
exports.encodeJWT = encodeJWT;
function decodeJWT(jwt) {
    const params = jwt.split(constants_1.JWT_DELIMITER);
    const header = decodeJSON(params[0]);
    const payload = decodeJSON(params[1]);
    const signature = decodeSig(params[2]);
    const data = from_string_1.fromString(params.slice(0, 2).join(constants_1.JWT_DELIMITER), constants_1.DATA_ENCODING);
    return { header, payload, signature, data };
}
exports.decodeJWT = decodeJWT;
//# sourceMappingURL=utils.js.map