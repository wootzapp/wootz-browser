import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "castVote" function.
 */
export type CastVoteParams = WithOverrides<{
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
    support: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "support";
    }>;
}>;
export declare const FN_SELECTOR: "0x56781388";
/**
 * Checks if the `castVote` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `castVote` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCastVoteSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isCastVoteSupported(["0x..."]);
 * ```
 */
export declare function isCastVoteSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "castVote" function.
 * @param options - The options for the castVote function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteParams({
 *  proposalId: ...,
 *  support: ...,
 * });
 * ```
 */
export declare function encodeCastVoteParams(options: CastVoteParams): `0x${string}`;
/**
 * Encodes the "castVote" function into a Hex string with its parameters.
 * @param options - The options for the castVote function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVote } from "thirdweb/extensions/vote";
 * const result = encodeCastVote({
 *  proposalId: ...,
 *  support: ...,
 * });
 * ```
 */
export declare function encodeCastVote(options: CastVoteParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "castVote" function on the contract.
 * @param options - The options for the "castVote" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { castVote } from "thirdweb/extensions/vote";
 *
 * const transaction = castVote({
 *  contract,
 *  proposalId: ...,
 *  support: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function castVote(options: BaseTransactionOptions<CastVoteParams | {
    asyncParams: () => Promise<CastVoteParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=castVote.d.ts.map