import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "castVoteWithReason" function.
 */
export type CastVoteWithReasonParams = WithOverrides<{
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
    support: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "support";
    }>;
    reason: AbiParameterToPrimitiveType<{
        type: "string";
        name: "reason";
    }>;
}>;
export declare const FN_SELECTOR: "0x7b3c71d3";
/**
 * Checks if the `castVoteWithReason` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `castVoteWithReason` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCastVoteWithReasonSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isCastVoteWithReasonSupported(["0x..."]);
 * ```
 */
export declare function isCastVoteWithReasonSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "castVoteWithReason" function.
 * @param options - The options for the castVoteWithReason function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReasonParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReasonParams({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReasonParams(options: CastVoteWithReasonParams): `0x${string}`;
/**
 * Encodes the "castVoteWithReason" function into a Hex string with its parameters.
 * @param options - The options for the castVoteWithReason function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReason } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReason({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReason(options: CastVoteWithReasonParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "castVoteWithReason" function on the contract.
 * @param options - The options for the "castVoteWithReason" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { castVoteWithReason } from "thirdweb/extensions/vote";
 *
 * const transaction = castVoteWithReason({
 *  contract,
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function castVoteWithReason(options: BaseTransactionOptions<CastVoteWithReasonParams | {
    asyncParams: () => Promise<CastVoteWithReasonParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=castVoteWithReason.d.ts.map