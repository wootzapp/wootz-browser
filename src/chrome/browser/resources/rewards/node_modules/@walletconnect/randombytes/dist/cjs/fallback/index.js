"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.randomBytes = void 0;
const tslib_1 = require("tslib");
const randombytes_1 = tslib_1.__importDefault(require("randombytes"));
function randomBytes(length) {
    return randombytes_1.default(length);
}
exports.randomBytes = randomBytes;
//# sourceMappingURL=index.js.map