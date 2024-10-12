import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "castVoteBySig" function.
 */
export type CastVoteBySigParams = WithOverrides<{
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
    support: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "support";
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
export declare const FN_SELECTOR: "0x3bccf4fd";
/**
 * Checks if the `castVoteBySig` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `castVoteBySig` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCastVoteBySigSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isCastVoteBySigSupported(["0x..."]);
 * ```
 */
export declare function isCastVoteBySigSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "castVoteBySig" function.
 * @param options - The options for the castVoteBySig function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteBySigParams } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteBySigParams({
 *  proposalId: ...,
 *  support: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeCastVoteBySigParams(options: CastVoteBySigParams): `0x${string}`;
/**
 * Encodes the "castVoteBySig" function into a Hex string with its parameters.
 * @param options - The options for the castVoteBySig function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeCastVoteBySig } from "thirdweb/extensions/vote";
 * const result = encodeCastVoteBySig({
 *  proposalId: ...,
 *  support: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeCastVoteBySig(options: CastVoteBySigParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "castVoteBySig" function on the contract.
 * @param options - The options for the "castVoteBySig" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { castVoteBySig } from "thirdweb/extensions/vote";
 *
 * const transaction = castVoteBySig({
 *  contract,
 *  proposalId: ...,
 *  support: ...,
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
export declare function castVoteBySig(options: BaseTransactionOptions<CastVoteBySigParams | {
    asyncParams: () => Promise<CastVoteBySigParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=castVoteBySig.d.ts.map