import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setVotingPeriod" function.
 */
export type SetVotingPeriodParams = WithOverrides<{
    newVotingPeriod: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "newVotingPeriod";
    }>;
}>;
export declare const FN_SELECTOR: "0xea0217cf";
/**
 * Checks if the `setVotingPeriod` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setVotingPeriod` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isSetVotingPeriodSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isSetVotingPeriodSupported(["0x..."]);
 * ```
 */
export declare function isSetVotingPeriodSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setVotingPeriod" function.
 * @param options - The options for the setVotingPeriod function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetVotingPeriodParams } from "thirdweb/extensions/vote";
 * const result = encodeSetVotingPeriodParams({
 *  newVotingPeriod: ...,
 * });
 * ```
 */
export declare function encodeSetVotingPeriodParams(options: SetVotingPeriodParams): `0x${string}`;
/**
 * Encodes the "setVotingPeriod" function into a Hex string with its parameters.
 * @param options - The options for the setVotingPeriod function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetVotingPeriod } from "thirdweb/extensions/vote";
 * const result = encodeSetVotingPeriod({
 *  newVotingPeriod: ...,
 * });
 * ```
 */
export declare function encodeSetVotingPeriod(options: SetVotingPeriodParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setVotingPeriod" function on the contract.
 * @param options - The options for the "setVotingPeriod" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setVotingPeriod } from "thirdweb/extensions/vote";
 *
 * const transaction = setVotingPeriod({
 *  contract,
 *  newVotingPeriod: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setVotingPeriod(options: BaseTransactionOptions<SetVotingPeriodParams | {
    asyncParams: () => Promise<SetVotingPeriodParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setVotingPeriod.d.ts.map