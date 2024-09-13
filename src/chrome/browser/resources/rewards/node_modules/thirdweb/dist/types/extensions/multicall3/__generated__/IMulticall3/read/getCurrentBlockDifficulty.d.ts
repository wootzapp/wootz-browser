import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x72425d9d";
/**
 * Checks if the `getCurrentBlockDifficulty` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getCurrentBlockDifficulty` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetCurrentBlockDifficultySupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetCurrentBlockDifficultySupported(["0x..."]);
 * ```
 */
export declare function isGetCurrentBlockDifficultySupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getCurrentBlockDifficulty function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetCurrentBlockDifficultyResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetCurrentBlockDifficultyResultResult("...");
 * ```
 */
export declare function decodeGetCurrentBlockDifficultyResult(result: Hex): bigint;
/**
 * Calls the "getCurrentBlockDifficulty" function on the contract.
 * @param options - The options for the getCurrentBlockDifficulty function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getCurrentBlockDifficulty } from "thirdweb/extensions/multicall3";
 *
 * const result = await getCurrentBlockDifficulty({
 *  contract,
 * });
 *
 * ```
 */
export declare function getCurrentBlockDifficulty(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getCurrentBlockDifficulty.d.ts.map