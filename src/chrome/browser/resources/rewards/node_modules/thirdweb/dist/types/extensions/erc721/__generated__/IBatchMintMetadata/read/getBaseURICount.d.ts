import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x63b45e2d";
/**
 * Checks if the `getBaseURICount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getBaseURICount` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isGetBaseURICountSupported } from "thirdweb/extensions/erc721";
 * const supported = isGetBaseURICountSupported(["0x..."]);
 * ```
 */
export declare function isGetBaseURICountSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getBaseURICount function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeGetBaseURICountResult } from "thirdweb/extensions/erc721";
 * const result = decodeGetBaseURICountResultResult("...");
 * ```
 */
export declare function decodeGetBaseURICountResult(result: Hex): bigint;
/**
 * Calls the "getBaseURICount" function on the contract.
 * @param options - The options for the getBaseURICount function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { getBaseURICount } from "thirdweb/extensions/erc721";
 *
 * const result = await getBaseURICount({
 *  contract,
 * });
 *
 * ```
 */
export declare function getBaseURICount(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getBaseURICount.d.ts.map