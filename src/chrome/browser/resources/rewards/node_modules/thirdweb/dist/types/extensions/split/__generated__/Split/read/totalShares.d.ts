import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3a98ef39";
/**
 * Checks if the `totalShares` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalShares` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isTotalSharesSupported } from "thirdweb/extensions/split";
 * const supported = isTotalSharesSupported(["0x..."]);
 * ```
 */
export declare function isTotalSharesSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalShares function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension SPLIT
 * @example
 * ```ts
 * import { decodeTotalSharesResult } from "thirdweb/extensions/split";
 * const result = decodeTotalSharesResultResult("...");
 * ```
 */
export declare function decodeTotalSharesResult(result: Hex): bigint;
/**
 * Calls the "totalShares" function on the contract.
 * @param options - The options for the totalShares function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { totalShares } from "thirdweb/extensions/split";
 *
 * const result = await totalShares({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalShares(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalShares.d.ts.map