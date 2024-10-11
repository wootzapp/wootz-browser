"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.hmacSha512Verify = exports.hmacSha512Sign = exports.hmacSha256Verify = exports.hmacSha256Sign = void 0;
const tslib_1 = require("tslib");
const helpers_1 = require("../helpers");
const node_1 = require("../lib/node");
function hmacSha256Sign(key, msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = node_1.nodeHmacSha256Sign(key, msg);
        return result;
    });
}
exports.hmacSha256Sign = hmacSha256Sign;
function hmacSha256Verify(key, msg, sig) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const expectedSig = node_1.nodeHmacSha256Sign(key, msg);
        const result = helpers_1.isConstantTime(expectedSig, sig);
        return result;
    });
}
exports.hmacSha256Verify = hmacSha256Verify;
function hmacSha512Sign(key, msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = node_1.nodeHmacSha512Sign(key, msg);
        return result;
    });
}
exports.hmacSha512Sign = hmacSha512Sign;
function hmacSha512Verify(key, msg, sig) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const expectedSig = node_1.nodeHmacSha512Sign(key, msg);
        const result = helpers_1.isConstantTime(expectedSig, sig);
        return result;
    });
}
exports.hmacSha512Verify = hmacSha512Verify;
//# sourceMappingURL=hmac.js.map