import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "castVoteWithReasonAndParamsBySig" function.
 */
export type CastVoteWithReasonAndParamsBySigParams = WithOverrides<{
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
    v: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "v";
    }>;
    r: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "r";
    }>;
    s: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "s";
    }>;
}>;
export declare const FN_SELECTOR: "0x03420181";
/**
 * Checks if the `castVoteWithReasonAndParamsBySig` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `castVoteWithReasonAndParamsBySig` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCastVoteWithReasonAndParamsBySigSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isCastVoteWithReasonAndParamsBySigSupported(["0x..."]);
 * ```
 */
export declare function isCastVoteWithReasonAndParamsBySigSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "castVoteWithReasonAndParamsBySig" function.
 * @param options - The options for the castVoteWithReasonAndParamsBySig function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReasonAndParamsBySigParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReasonAndParamsBySigParams({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReasonAndParamsBySigParams(options: CastVoteWithReasonAndParamsBySigParams): `0x${string}`;
/**
 * Encodes the "castVoteWithReasonAndParamsBySig" function into a Hex string with its parameters.
 * @param options - The options for the castVoteWithReasonAndParamsBySig function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteWithReasonAndParamsBySig } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteWithReasonAndParamsBySig({
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeCastVoteWithReasonAndParamsBySig(options: CastVoteWithReasonAndParamsBySigParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "castVoteWithReasonAndParamsBySig" function on the contract.
 * @param options - The options for the "castVoteWithReasonAndParamsBySig" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { castVoteWithReasonAndParamsBySig } from "thirdweb/extensions/vote";
 *
 * const transaction = castVoteWithReasonAndParamsBySig({
 *  contract,
 *  proposalId: ...,
 *  support: ...,
 *  reason: ...,
 *  params: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function castVoteWithReasonAndParamsBySig(options: BaseTransactionOptions<CastVoteWithReasonAndParamsBySigParams | {
    asyncParams: () => Promise<CastVoteWithReasonAndParamsBySigParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=castVoteWithReasonAndParamsBySig.d.ts.map