"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isNameSupported = void 0;
exports.name = name;
const withCache_js_1 = require("../../../utils/promise/withCache.js");
const name_js_1 = require("../__generated__/IContractMetadata/read/name.js");
var name_js_2 = require("../__generated__/IContractMetadata/read/name.js");
Object.defineProperty(exports, "isNameSupported", { enumerable: true, get: function () { return name_js_2.isNameSupported; } });
/**
 * Retrieves the name associated with the given contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the name associated with the contract.
 * @extension COMMON
 * @example
 * ```ts
 * import { name } from "thirdweb/extensions/common";
 *
 * const contractName = await name({ contract });
 * ```
 */
async function name(options) {
    return (0, withCache_js_1.withCache)(() => (0, name_js_1.name)(options), {
        cacheKey: `${options.contract.chain.id}:${options.contract.address}:name`,
        // can never change, so cache forever
        cacheTime: Number.POSITIVE_INFINITY,
    });
}
//# sourceMappingURL=name.js.map