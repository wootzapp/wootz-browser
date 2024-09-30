"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isJWT = isJWT;
function isJWT(str) {
    return str.split(".").length === 3;
}
//# sourceMappingURL=is-jwt.js.map