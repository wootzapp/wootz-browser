import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x42cbb15c";
/**
 * Checks if the `getBlockNumber` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getBlockNumber` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetBlockNumberSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetBlockNumberSupported(["0x..."]);
 * ```
 */
export declare function isGetBlockNumberSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getBlockNumber function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetBlockNumberResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetBlockNumberResultResult("...");
 * ```
 */
export declare function decodeGetBlockNumberResult(result: Hex): bigint;
/**
 * Calls the "getBlockNumber" function on the contract.
 * @param options - The options for the getBlockNumber function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getBlockNumber } from "thirdweb/extensions/multicall3";
 *
 * const result = await getBlockNumber({
 *  contract,
 * });
 *
 * ```
 */
export declare function getBlockNumber(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getBlockNumber.d.ts.map