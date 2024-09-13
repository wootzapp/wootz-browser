import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x16002f4a";
/**
 * Checks if the `totalAuctions` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalAuctions` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isTotalAuctionsSupported } from "thirdweb/extensions/marketplace";
 * const supported = isTotalAuctionsSupported(["0x..."]);
 * ```
 */
export declare function isTotalAuctionsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalAuctions function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeTotalAuctionsResult } from "thirdweb/extensions/marketplace";
 * const result = decodeTotalAuctionsResultResult("...");
 * ```
 */
export declare function decodeTotalAuctionsResult(result: Hex): bigint;
/**
 * Calls the "totalAuctions" function on the contract.
 * @param options - The options for the totalAuctions function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { totalAuctions } from "thirdweb/extensions/marketplace";
 *
 * const result = await totalAuctions({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalAuctions(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalAuctions.d.ts.map