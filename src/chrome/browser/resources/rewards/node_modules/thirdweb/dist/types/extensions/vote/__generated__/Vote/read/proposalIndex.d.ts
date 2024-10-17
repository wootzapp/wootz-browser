import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x5977e0f2";
/**
 * Checks if the `proposalIndex` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proposalIndex` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposalIndexSupported } from "thirdweb/extensions/vote";
 * const supported = isProposalIndexSupported(["0x..."]);
 * ```
 */
export declare function isProposalIndexSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the proposalIndex function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeProposalIndexResult } from "thirdweb/extensions/vote";
 * const result = decodeProposalIndexResultResult("...");
 * ```
 */
export declare function decodeProposalIndexResult(result: Hex): bigint;
/**
 * Calls the "proposalIndex" function on the contract.
 * @param options - The options for the proposalIndex function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { proposalIndex } from "thirdweb/extensions/vote";
 *
 * const result = await proposalIndex({
 *  contract,
 * });
 *
 * ```
 */
export declare function proposalIndex(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=proposalIndex.d.ts.map