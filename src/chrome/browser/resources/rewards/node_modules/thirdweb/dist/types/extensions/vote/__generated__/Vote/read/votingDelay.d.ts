import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3932abb1";
/**
 * Checks if the `votingDelay` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `votingDelay` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isVotingDelaySupported } from "thirdweb/extensions/vote";
 * const supported = isVotingDelaySupported(["0x..."]);
 * ```
 */
export declare function isVotingDelaySupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the votingDelay function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeVotingDelayResult } from "thirdweb/extensions/vote";
 * const result = decodeVotingDelayResultResult("...");
 * ```
 */
export declare function decodeVotingDelayResult(result: Hex): bigint;
/**
 * Calls the "votingDelay" function on the contract.
 * @param options - The options for the votingDelay function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { votingDelay } from "thirdweb/extensions/vote";
 *
 * const result = await votingDelay({
 *  contract,
 * });
 *
 * ```
 */
export declare function votingDelay(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=votingDelay.d.ts.map