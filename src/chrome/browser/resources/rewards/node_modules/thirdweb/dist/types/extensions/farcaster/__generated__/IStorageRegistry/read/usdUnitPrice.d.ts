import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x40df0ba0";
/**
 * Checks if the `usdUnitPrice` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `usdUnitPrice` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isUsdUnitPriceSupported } from "thirdweb/extensions/farcaster";
 * const supported = isUsdUnitPriceSupported(["0x..."]);
 * ```
 */
export declare function isUsdUnitPriceSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the usdUnitPrice function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeUsdUnitPriceResult } from "thirdweb/extensions/farcaster";
 * const result = decodeUsdUnitPriceResultResult("...");
 * ```
 */
export declare function decodeUsdUnitPriceResult(result: Hex): bigint;
/**
 * Calls the "usdUnitPrice" function on the contract.
 * @param options - The options for the usdUnitPrice function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { usdUnitPrice } from "thirdweb/extensions/farcaster";
 *
 * const result = await usdUnitPrice({
 *  contract,
 * });
 *
 * ```
 */
export declare function usdUnitPrice(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=usdUnitPrice.d.ts.map