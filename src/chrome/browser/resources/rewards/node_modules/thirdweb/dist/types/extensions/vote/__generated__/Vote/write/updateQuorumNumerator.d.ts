import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "updateQuorumNumerator" function.
 */
export type UpdateQuorumNumeratorParams = WithOverrides<{
    newQuorumNumerator: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "newQuorumNumerator";
    }>;
}>;
export declare const FN_SELECTOR: "0x06f3f9e6";
/**
 * Checks if the `updateQuorumNumerator` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `updateQuorumNumerator` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isUpdateQuorumNumeratorSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isUpdateQuorumNumeratorSupported(["0x..."]);
 * ```
 */
export declare function isUpdateQuorumNumeratorSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "updateQuorumNumerator" function.
 * @param options - The options for the updateQuorumNumerator function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeUpdateQuorumNumeratorParams } from "thirdweb/extensions/vote";
 * const result = encodeUpdateQuorumNumeratorParams({
 *  newQuorumNumerator: ...,
 * });
 * ```
 */
export declare function encodeUpdateQuorumNumeratorParams(options: UpdateQuorumNumeratorParams): `0x${string}`;
/**
 * Encodes the "updateQuorumNumerator" function into a Hex string with its parameters.
 * @param options - The options for the updateQuorumNumerator function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeUpdateQuorumNumerator } from "thirdweb/extensions/vote";
 * const result = encodeUpdateQuorumNumerator({
 *  newQuorumNumerator: ...,
 * });
 * ```
 */
export declare function encodeUpdateQuorumNumerator(options: UpdateQuorumNumeratorParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "updateQuorumNumerator" function on the contract.
 * @param options - The options for the "updateQuorumNumerator" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { updateQuorumNumerator } from "thirdweb/extensions/vote";
 *
 * const transaction = updateQuorumNumerator({
 *  contract,
 *  newQuorumNumerator: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function updateQuorumNumerator(options: BaseTransactionOptions<UpdateQuorumNumeratorParams | {
    asyncParams: () => Promise<UpdateQuorumNumeratorParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=updateQuorumNumerator.d.ts.map