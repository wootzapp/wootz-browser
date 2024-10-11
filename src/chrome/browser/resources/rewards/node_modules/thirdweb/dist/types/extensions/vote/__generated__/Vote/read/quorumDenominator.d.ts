import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x97c3d334";
/**
 * Checks if the `quorumDenominator` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quorumDenominator` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isQuorumDenominatorSupported } from "thirdweb/extensions/vote";
 * const supported = isQuorumDenominatorSupported(["0x..."]);
 * ```
 */
export declare function isQuorumDenominatorSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the quorumDenominator function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeQuorumDenominatorResult } from "thirdweb/extensions/vote";
 * const result = decodeQuorumDenominatorResultResult("...");
 * ```
 */
export declare function decodeQuorumDenominatorResult(result: Hex): bigint;
/**
 * Calls the "quorumDenominator" function on the contract.
 * @param options - The options for the quorumDenominator function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { quorumDenominator } from "thirdweb/extensions/vote";
 *
 * const result = await quorumDenominator({
 *  contract,
 * });
 *
 * ```
 */
export declare function quorumDenominator(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=quorumDenominator.d.ts.map