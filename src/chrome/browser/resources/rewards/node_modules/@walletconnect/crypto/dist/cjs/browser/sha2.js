"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ripemd160 = exports.sha512 = exports.sha256 = void 0;
const tslib_1 = require("tslib");
const browser_1 = require("../lib/browser");
function sha256(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = yield browser_1.browserSha256(msg);
        return result;
    });
}
exports.sha256 = sha256;
function sha512(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = yield browser_1.browserSha512(msg);
        return result;
    });
}
exports.sha512 = sha512;
function ripemd160(msg) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        throw new Error("Not supported for Browser async methods, use sync instead!");
    });
}
exports.ripemd160 = ripemd160;
//# sourceMappingURL=sha2.js.map