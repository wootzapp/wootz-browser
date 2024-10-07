"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isSymbolSupported = void 0;
exports.symbol = symbol;
const withCache_js_1 = require("../../../utils/promise/withCache.js");
const symbol_js_1 = require("../__generated__/IContractMetadata/read/symbol.js");
var symbol_js_2 = require("../__generated__/IContractMetadata/read/symbol.js");
Object.defineProperty(exports, "isSymbolSupported", { enumerable: true, get: function () { return symbol_js_2.isSymbolSupported; } });
/**
 * Retrieves the name associated with the given contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the name associated with the contract.
 * @extension COMMON
 * @example
 * ```ts
 * import { symbol } from "thirdweb/extensions/common";
 *
 * const contractSymbol = await symbol({ contract });
 * ```
 */
async function symbol(options) {
    return (0, withCache_js_1.withCache)(() => (0, symbol_js_1.symbol)(options), {
        cacheKey: `${options.contract.chain.id}:${options.contract.address}:symbol`,
        // can never change, so cache forever
        cacheTime: Number.POSITIVE_INFINITY,
    });
}
//# sourceMappingURL=symbol.js.map