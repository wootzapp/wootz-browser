import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x0f28c97d";
/**
 * Checks if the `getCurrentBlockTimestamp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getCurrentBlockTimestamp` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetCurrentBlockTimestampSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetCurrentBlockTimestampSupported(["0x..."]);
 * ```
 */
export declare function isGetCurrentBlockTimestampSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getCurrentBlockTimestamp function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetCurrentBlockTimestampResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetCurrentBlockTimestampResultResult("...");
 * ```
 */
export declare function decodeGetCurrentBlockTimestampResult(result: Hex): bigint;
/**
 * Calls the "getCurrentBlockTimestamp" function on the contract.
 * @param options - The options for the getCurrentBlockTimestamp function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getCurrentBlockTimestamp } from "thirdweb/extensions/multicall3";
 *
 * const result = await getCurrentBlockTimestamp({
 *  contract,
 * });
 *
 * ```
 */
export declare function getCurrentBlockTimestamp(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getCurrentBlockTimestamp.d.ts.map