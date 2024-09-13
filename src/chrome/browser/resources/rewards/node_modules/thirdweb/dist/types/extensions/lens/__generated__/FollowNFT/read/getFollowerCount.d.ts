import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x7829ae4a";
/**
 * Checks if the `getFollowerCount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getFollowerCount` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetFollowerCountSupported } from "thirdweb/extensions/lens";
 * const supported = isGetFollowerCountSupported(["0x..."]);
 * ```
 */
export declare function isGetFollowerCountSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getFollowerCount function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetFollowerCountResult } from "thirdweb/extensions/lens";
 * const result = decodeGetFollowerCountResultResult("...");
 * ```
 */
export declare function decodeGetFollowerCountResult(result: Hex): bigint;
/**
 * Calls the "getFollowerCount" function on the contract.
 * @param options - The options for the getFollowerCount function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getFollowerCount } from "thirdweb/extensions/lens";
 *
 * const result = await getFollowerCount({
 *  contract,
 * });
 *
 * ```
 */
export declare function getFollowerCount(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getFollowerCount.d.ts.map