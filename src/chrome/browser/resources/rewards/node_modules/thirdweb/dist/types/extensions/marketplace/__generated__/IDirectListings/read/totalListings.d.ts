import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xc78b616c";
/**
 * Checks if the `totalListings` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalListings` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isTotalListingsSupported } from "thirdweb/extensions/marketplace";
 * const supported = isTotalListingsSupported(["0x..."]);
 * ```
 */
export declare function isTotalListingsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalListings function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeTotalListingsResult } from "thirdweb/extensions/marketplace";
 * const result = decodeTotalListingsResultResult("...");
 * ```
 */
export declare function decodeTotalListingsResult(result: Hex): bigint;
/**
 * Calls the "totalListings" function on the contract.
 * @param options - The options for the totalListings function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { totalListings } from "thirdweb/extensions/marketplace";
 *
 * const result = await totalListings({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalListings(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalListings.d.ts.map