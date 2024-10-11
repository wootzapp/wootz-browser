"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getFunctionId = getFunctionId;
const random_js_1 = require("./random.js");
const functionIdCache = new Map();
/**
 * Retrieves the unique identifier for a given function.
 * If the function has been previously cached, the cached identifier is returned.
 * Otherwise, a new identifier is generated using the function's string representation.
 * @param fn - The function for which to retrieve the identifier.
 * @returns The unique identifier for the function.
 * @internal
 */
function getFunctionId(fn) {
    if (functionIdCache.has(fn)) {
        // biome-ignore lint/style/noNonNullAssertion: the `has` above ensures that this will always be set
        return functionIdCache.get(fn);
    }
    const id = (0, random_js_1.randomBytesHex)();
    functionIdCache.set(fn, id);
    return id;
}
//# sourceMappingURL=function-id.js.map