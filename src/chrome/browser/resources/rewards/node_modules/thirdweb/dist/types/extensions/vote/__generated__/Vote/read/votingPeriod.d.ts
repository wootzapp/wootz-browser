import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x02a251a3";
/**
 * Checks if the `votingPeriod` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `votingPeriod` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isVotingPeriodSupported } from "thirdweb/extensions/vote";
 * const supported = isVotingPeriodSupported(["0x..."]);
 * ```
 */
export declare function isVotingPeriodSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the votingPeriod function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeVotingPeriodResult } from "thirdweb/extensions/vote";
 * const result = decodeVotingPeriodResultResult("...");
 * ```
 */
export declare function decodeVotingPeriodResult(result: Hex): bigint;
/**
 * Calls the "votingPeriod" function on the contract.
 * @param options - The options for the votingPeriod function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { votingPeriod } from "thirdweb/extensions/vote";
 *
 * const result = await votingPeriod({
 *  contract,
 * });
 *
 * ```
 */
export declare function votingPeriod(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=votingPeriod.d.ts.map