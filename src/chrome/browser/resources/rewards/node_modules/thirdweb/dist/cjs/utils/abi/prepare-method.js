"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.prepareMethod = prepareMethod;
const abitype_1 = require("abitype");
const viem_1 = require("viem");
const lru_js_1 = require("../caching/lru.js");
const json_js_1 = require("../json.js");
const prepareMethodCache = new lru_js_1.LruMap(4096);
/**
 * Prepares a method for usage by converting it into a prepared method object.
 * @param method The method to prepare.
 * @returns The prepared method object.
 * @example
 * ```ts
 * import { prepareMethod } from "thirdweb/utils";
 * const method = "function transfer(address to, uint256 value)";
 * const preparedMethod = prepareMethod(method);
 * ```
 * @contract
 */
function prepareMethod(method) {
    const key = typeof method === "string" ? method : (0, json_js_1.stringify)(method);
    if (prepareMethodCache.has(key)) {
        return prepareMethodCache.get(key);
    }
    const abiFn = typeof method === "string"
        ? // @ts-expect-error - we're sure it's a string...
            (0, abitype_1.parseAbiItem)(method)
        : method;
    // encode the method signature
    const sig = (0, viem_1.toFunctionSelector)(abiFn);
    const ret = [sig, abiFn.inputs, abiFn.outputs];
    prepareMethodCache.set(key, ret);
    return ret;
}
//# sourceMappingURL=prepare-method.js.map