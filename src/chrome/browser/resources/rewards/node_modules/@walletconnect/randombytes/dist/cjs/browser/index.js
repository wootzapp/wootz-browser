"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.randomBytes = void 0;
const tslib_1 = require("tslib");
const env = tslib_1.__importStar(require("@walletconnect/environment"));
function randomBytes(length) {
    const browserCrypto = env.getBrowerCrypto();
    return browserCrypto.getRandomValues(new Uint8Array(length));
}
exports.randomBytes = randomBytes;
//# sourceMappingURL=index.js.map