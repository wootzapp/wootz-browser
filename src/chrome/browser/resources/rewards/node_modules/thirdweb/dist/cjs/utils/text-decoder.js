"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.cachedTextDecoder = cachedTextDecoder;
let textDecoder;
/**
 * Re-using the same textdecoder is faster.
 *
 * @returns
 * @internal
 */
function cachedTextDecoder() {
    if (!textDecoder) {
        textDecoder = new TextDecoder();
    }
    return textDecoder;
}
//# sourceMappingURL=text-decoder.js.map