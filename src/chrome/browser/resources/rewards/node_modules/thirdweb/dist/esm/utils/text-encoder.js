let textEncoder;
/**
 * Re-using the same textencoder is faster.
 *
 * @returns
 * @internal
 */
export function cachedTextEncoder() {
    if (!textEncoder) {
        textEncoder = new TextEncoder();
    }
    return textEncoder;
}
//# sourceMappingURL=text-encoder.js.map