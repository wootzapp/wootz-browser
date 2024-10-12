import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x86d516e8";
/**
 * Checks if the `getCurrentBlockGasLimit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getCurrentBlockGasLimit` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetCurrentBlockGasLimitSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetCurrentBlockGasLimitSupported(["0x..."]);
 * ```
 */
export declare function isGetCurrentBlockGasLimitSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getCurrentBlockGasLimit function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetCurrentBlockGasLimitResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetCurrentBlockGasLimitResultResult("...");
 * ```
 */
export declare function decodeGetCurrentBlockGasLimitResult(result: Hex): bigint;
/**
 * Calls the "getCurrentBlockGasLimit" function on the contract.
 * @param options - The options for the getCurrentBlockGasLimit function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getCurrentBlockGasLimit } from "thirdweb/extensions/multicall3";
 *
 * const result = await getCurrentBlockGasLimit({
 *  contract,
 * });
 *
 * ```
 */
export declare function getCurrentBlockGasLimit(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getCurrentBlockGasLimit.d.ts.map