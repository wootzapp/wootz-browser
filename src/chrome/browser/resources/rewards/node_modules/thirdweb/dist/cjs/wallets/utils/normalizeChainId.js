"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.normalizeChainId = normalizeChainId;
const hex_js_1 = require("../../utils/encoding/hex.js");
/**
 * @internal
 */
function normalizeChainId(chainId) {
    if (typeof chainId === "number") {
        return chainId;
    }
    if ((0, hex_js_1.isHex)(chainId)) {
        return (0, hex_js_1.hexToNumber)(chainId);
    }
    if (typeof chainId === "bigint") {
        return Number(chainId);
    }
    return Number.parseInt(chainId, 10);
}
//# sourceMappingURL=normalizeChainId.js.map