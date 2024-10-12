import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3e64a696";
/**
 * Checks if the `getBasefee` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getBasefee` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetBasefeeSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetBasefeeSupported(["0x..."]);
 * ```
 */
export declare function isGetBasefeeSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getBasefee function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetBasefeeResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetBasefeeResultResult("...");
 * ```
 */
export declare function decodeGetBasefeeResult(result: Hex): bigint;
/**
 * Calls the "getBasefee" function on the contract.
 * @param options - The options for the getBasefee function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getBasefee } from "thirdweb/extensions/multicall3";
 *
 * const result = await getBasefee({
 *  contract,
 * });
 *
 * ```
 */
export declare function getBasefee(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getBasefee.d.ts.map