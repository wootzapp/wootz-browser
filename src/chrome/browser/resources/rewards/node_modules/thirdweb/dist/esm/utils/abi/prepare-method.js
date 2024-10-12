import { parseAbiItem } from "abitype";
import { toFunctionSelector } from "viem";
import { LruMap } from "../caching/lru.js";
import { stringify } from "../json.js";
const prepareMethodCache = new LruMap(4096);
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
export function prepareMethod(method) {
    const key = typeof method === "string" ? method : stringify(method);
    if (prepareMethodCache.has(key)) {
        return prepareMethodCache.get(key);
    }
    const abiFn = typeof method === "string"
        ? // @ts-expect-error - we're sure it's a string...
            parseAbiItem(method)
        : method;
    // encode the method signature
    const sig = toFunctionSelector(abiFn);
    const ret = [sig, abiFn.inputs, abiFn.outputs];
    prepareMethodCache.set(key, ret);
    return ret;
}
//# sourceMappingURL=prepare-method.js.map