import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x95d89b41";
/**
 * Checks if the `symbol` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `symbol` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSymbolSupported } from "thirdweb/extensions/common";
 * const supported = isSymbolSupported(["0x..."]);
 * ```
 */
export declare function isSymbolSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the symbol function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension COMMON
 * @example
 * ```ts
 * import { decodeSymbolResult } from "thirdweb/extensions/common";
 * const result = decodeSymbolResultResult("...");
 * ```
 */
export declare function decodeSymbolResult(result: Hex): string;
/**
 * Calls the "symbol" function on the contract.
 * @param options - The options for the symbol function.
 * @returns The parsed result of the function call.
 * @extension COMMON
 * @example
 * ```ts
 * import { symbol } from "thirdweb/extensions/common";
 *
 * const result = await symbol({
 *  contract,
 * });
 *
 * ```
 */
export declare function symbol(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=symbol.d.ts.map