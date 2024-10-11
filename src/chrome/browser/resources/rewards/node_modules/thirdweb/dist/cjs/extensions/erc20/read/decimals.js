"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isDecimalsSupported = void 0;
exports.decimals = decimals;
const withCache_js_1 = require("../../../utils/promise/withCache.js");
const decimals_js_1 = require("../__generated__/IERC20/read/decimals.js");
var decimals_js_2 = require("../__generated__/IERC20/read/decimals.js");
Object.defineProperty(exports, "isDecimalsSupported", { enumerable: true, get: function () { return decimals_js_2.isDecimalsSupported; } });
/**
 * Retrieves the number of decimal places for a given ERC20 contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the number of decimal places.
 * @extension ERC20
 * @example
 * ```ts
 * import { decimals } from "thirdweb/extensions/erc20";
 *
 * const tokenDecimals = await decimals({ contract });
 * ```
 */
async function decimals(options) {
    return (0, withCache_js_1.withCache)(() => (0, decimals_js_1.decimals)(options), {
        cacheKey: `${options.contract.chain.id}:${options.contract.address}:decimals`,
        // can never change, so cache forever
        cacheTime: Number.POSITIVE_INFINITY,
    });
}
//# sourceMappingURL=decimals.js.map