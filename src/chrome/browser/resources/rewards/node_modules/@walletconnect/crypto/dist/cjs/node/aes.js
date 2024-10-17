"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.aesCbcDecrypt = exports.aesCbcEncrypt = void 0;
const tslib_1 = require("tslib");
const node_1 = require("../lib/node");
function aesCbcEncrypt(iv, key, data) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = node_1.nodeAesEncrypt(iv, key, data);
        return result;
    });
}
exports.aesCbcEncrypt = aesCbcEncrypt;
function aesCbcDecrypt(iv, key, data) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        const result = node_1.nodeAesDecrypt(iv, key, data);
        return result;
    });
}
exports.aesCbcDecrypt = aesCbcDecrypt;
//# sourceMappingURL=aes.js.map