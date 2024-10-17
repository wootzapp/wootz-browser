import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setVotingDelay" function.
 */
export type SetVotingDelayParams = WithOverrides<{
    newVotingDelay: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "newVotingDelay";
    }>;
}>;
export declare const FN_SELECTOR: "0x70b0f660";
/**
 * Checks if the `setVotingDelay` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setVotingDelay` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isSetVotingDelaySupported } from "thirdweb/extensions/vote";
 *
 * const supported = isSetVotingDelaySupported(["0x..."]);
 * ```
 */
export declare function isSetVotingDelaySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setVotingDelay" function.
 * @param options - The options for the setVotingDelay function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetVotingDelayParams } from "thirdweb/extensions/vote";
 * const result = encodeSetVotingDelayParams({
 *  newVotingDelay: ...,
 * });
 * ```
 */
export declare function encodeSetVotingDelayParams(options: SetVotingDelayParams): `0x${string}`;
/**
 * Encodes the "setVotingDelay" function into a Hex string with its parameters.
 * @param options - The options for the setVotingDelay function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetVotingDelay } from "thirdweb/extensions/vote";
 * const result = encodeSetVotingDelay({
 *  newVotingDelay: ...,
 * });
 * ```
 */
export declare function encodeSetVotingDelay(options: SetVotingDelayParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setVotingDelay" function on the contract.
 * @param options - The options for the "setVotingDelay" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setVotingDelay } from "thirdweb/extensions/vote";
 *
 * const transaction = setVotingDelay({
 *  contract,
 *  newVotingDelay: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setVotingDelay(options: BaseTransactionOptions<SetVotingDelayParams | {
    asyncParams: () => Promise<SetVotingDelayParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setVotingDelay.d.ts.map