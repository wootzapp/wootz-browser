import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "castVoteWithReasonAndParams" function.
 */
export type CastVoteWithReasonAndParamsParams = WithOverrides<{
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
    params: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "params";
    }>;
}>;
export declare const FN_SELECTOR: "0x5f398a14";
/**
 * Checks if the `castVoteWithReasonAndParams` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `castVoteWithReasonAndParams` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCastVoteWithReasonAndParamsSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isCastVoteWithReasonAndParamsSupported(["0x..."]);
 * ```
 */
export declare function isCastVoteWithReasonAndParamsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "castVoteWithReasonAndParams" function.
 * @param options - The options for the castVoteWithReasonAndParams function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReasonAndParamsParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReasonAndParamsParams({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReasonAndParamsParams(options: CastVoteWithReasonAndParamsParams): `0x${string}`;
/**
 * Encodes the "castVoteWithReasonAndParams" function into a Hex string with its parameters.
 * @param options - The options for the castVoteWithReasonAndParams function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReasonAndParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReasonAndParams({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReasonAndParams(options: CastVoteWithReasonAndParamsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "castVoteWithReasonAndParams" function on the contract.
 * @param options - The options for the "castVoteWithReasonAndParams" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { castVoteWithReasonAndParams } from "thirdweb/extensions/vote";
 *
 * const transaction = castVoteWithReasonAndParams({
 *  contract,
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function castVoteWithReasonAndParams(options: BaseTransactionOptions<CastVoteWithReasonAndParamsParams | {
    asyncParams: () => Promise<CastVoteWithReasonAndParamsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=castVoteWithReasonAndParams.d.ts.map