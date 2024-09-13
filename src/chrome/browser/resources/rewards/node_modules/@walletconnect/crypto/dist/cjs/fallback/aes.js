"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.aesCbcDecrypt = exports.aesCbcEncrypt = void 0;
const tslib_1 = require("tslib");
const fallback_1 = require("../lib/fallback");
function aesCbcEncrypt(iv, key, data) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = fallback_1.fallbackAesEncrypt(iv, key, data);
        return result;
    });
}
exports.aesCbcEncrypt = aesCbcEncrypt;
function aesCbcDecrypt(iv, key, data) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = fallback_1.fallbackAesDecrypt(iv, key, data);
        return result;
    });
}
exports.aesCbcDecrypt = aesCbcDecrypt;
//# sourceMappingURL=aes.js.map