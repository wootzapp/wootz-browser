import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "proposalDeadline" function.
 */
export type ProposalDeadlineParams = {
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
};
export declare const FN_SELECTOR: "0xc01f9e37";
/**
 * Checks if the `proposalDeadline` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proposalDeadline` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposalDeadlineSupported } from "thirdweb/extensions/vote";
 * const supported = isProposalDeadlineSupported(["0x..."]);
 * ```
 */
export declare function isProposalDeadlineSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "proposalDeadline" function.
 * @param options - The options for the proposalDeadline function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalDeadlineParams } from "thirdweb/extensions/vote";
 * const result = encodeProposalDeadlineParams({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalDeadlineParams(options: ProposalDeadlineParams): `0x${string}`;
/**
 * Encodes the "proposalDeadline" function into a Hex string with its parameters.
 * @param options - The options for the proposalDeadline function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalDeadline } from "thirdweb/extensions/vote";
 * const result = encodeProposalDeadline({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalDeadline(options: ProposalDeadlineParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the proposalDeadline function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeProposalDeadlineResult } from "thirdweb/extensions/vote";
 * const result = decodeProposalDeadlineResultResult("...");
 * ```
 */
export declare function decodeProposalDeadlineResult(result: Hex): bigint;
/**
 * Calls the "proposalDeadline" function on the contract.
 * @param options - The options for the proposalDeadline function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { proposalDeadline } from "thirdweb/extensions/vote";
 *
 * const result = await proposalDeadline({
 *  contract,
 *  proposalId: ...,
 * });
 *
 * ```
 */
export declare function proposalDeadline(options: BaseTransactionOptions<ProposalDeadlineParams>): Promise<bigint>;
//# sourceMappingURL=proposalDeadline.d.ts.map