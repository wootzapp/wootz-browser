import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xb58131b0";
/**
 * Checks if the `proposalThreshold` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proposalThreshold` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposalThresholdSupported } from "thirdweb/extensions/vote";
 * const supported = isProposalThresholdSupported(["0x..."]);
 * ```
 */
export declare function isProposalThresholdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the proposalThreshold function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeProposalThresholdResult } from "thirdweb/extensions/vote";
 * const result = decodeProposalThresholdResultResult("...");
 * ```
 */
export declare function decodeProposalThresholdResult(result: Hex): bigint;
/**
 * Calls the "proposalThreshold" function on the contract.
 * @param options - The options for the proposalThreshold function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { proposalThreshold } from "thirdweb/extensions/vote";
 *
 * const result = await proposalThreshold({
 *  contract,
 * });
 *
 * ```
 */
export declare function proposalThreshold(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=proposalThreshold.d.ts.map