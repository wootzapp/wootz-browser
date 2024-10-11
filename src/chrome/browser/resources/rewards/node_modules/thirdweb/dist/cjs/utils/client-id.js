"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.computeClientIdFromSecretKey = computeClientIdFromSecretKey;
const lru_js_1 = require("./caching/lru.js");
const to_bytes_js_1 = require("./encoding/to-bytes.js");
const sha256_js_1 = require("./hashing/sha256.js");
const cache = new lru_js_1.LruMap(4096);
/**
 * @param secretKey - the secret key to compute the client id from
 * @returns the 32 char hex client id
 * @internal
 */
function computeClientIdFromSecretKey(secretKey) {
    if (cache.has(secretKey)) {
        return cache.get(secretKey);
    }
    // we slice off the leading `0x` and then take the first 32 chars
    const cId = (0, sha256_js_1.sha256)((0, to_bytes_js_1.stringToBytes)(secretKey)).slice(2, 34);
    cache.set(secretKey, cId);
    return cId;
}
//# sourceMappingURL=client-id.js.map