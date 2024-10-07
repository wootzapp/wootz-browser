let textDecoder;
/**
 * Re-using the same textdecoder is faster.
 *
 * @returns
 * @internal
 */
export function cachedTextDecoder() {
    if (!textDecoder) {
        textDecoder = new TextDecoder();
    }
    return textDecoder;
}
//# sourceMappingURL=text-decoder.js.map