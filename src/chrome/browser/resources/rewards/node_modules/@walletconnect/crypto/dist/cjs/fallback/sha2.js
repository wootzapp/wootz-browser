"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ripemd160 = exports.sha512 = exports.sha256 = void 0;
const tslib_1 = require("tslib");
const fallback_1 = require("../lib/fallback");
function sha256(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = fallback_1.fallbackSha256(msg);
        return result;
    });
}
exports.sha256 = sha256;
function sha512(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = fallback_1.fallbackSha512(msg);
        return result;
    });
}
exports.sha512 = sha512;
function ripemd160(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = fallback_1.fallbackRipemd160(msg);
        return result;
    });
}
exports.ripemd160 = ripemd160;
//# sourceMappingURL=sha2.js.map