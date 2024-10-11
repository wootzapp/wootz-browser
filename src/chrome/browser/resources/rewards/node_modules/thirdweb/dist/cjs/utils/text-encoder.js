"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.cachedTextEncoder = cachedTextEncoder;
let textEncoder;
/**
 * Re-using the same textencoder is faster.
 *
 * @returns
 * @internal
 */
function cachedTextEncoder() {
    if (!textEncoder) {
        textEncoder = new TextEncoder();
    }
    return textEncoder;
}
//# sourceMappingURL=text-encoder.js.map