import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "proposalVotes" function.
 */
export type ProposalVotesParams = {
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
};
export declare const FN_SELECTOR: "0x544ffc9c";
/**
 * Checks if the `proposalVotes` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proposalVotes` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposalVotesSupported } from "thirdweb/extensions/vote";
 * const supported = isProposalVotesSupported(["0x..."]);
 * ```
 */
export declare function isProposalVotesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "proposalVotes" function.
 * @param options - The options for the proposalVotes function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalVotesParams } from "thirdweb/extensions/vote";
 * const result = encodeProposalVotesParams({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalVotesParams(options: ProposalVotesParams): `0x${string}`;
/**
 * Encodes the "proposalVotes" function into a Hex string with its parameters.
 * @param options - The options for the proposalVotes function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalVotes } from "thirdweb/extensions/vote";
 * const result = encodeProposalVotes({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalVotes(options: ProposalVotesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the proposalVotes function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeProposalVotesResult } from "thirdweb/extensions/vote";
 * const result = decodeProposalVotesResultResult("...");
 * ```
 */
export declare function decodeProposalVotesResult(result: Hex): readonly [bigint, bigint, bigint];
/**
 * Calls the "proposalVotes" function on the contract.
 * @param options - The options for the proposalVotes function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { proposalVotes } from "thirdweb/extensions/vote";
 *
 * const result = await proposalVotes({
 *  contract,
 *  proposalId: ...,
 * });
 *
 * ```
 */
export declare function proposalVotes(options: BaseTransactionOptions<ProposalVotesParams>): Promise<readonly [bigint, bigint, bigint]>;
//# sourceMappingURL=proposalVotes.d.ts.map