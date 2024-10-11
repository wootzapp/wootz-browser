import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setProposalThreshold" function.
 */
export type SetProposalThresholdParams = WithOverrides<{
    newProposalThreshold: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "newProposalThreshold";
    }>;
}>;
export declare const FN_SELECTOR: "0xece40cc1";
/**
 * Checks if the `setProposalThreshold` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setProposalThreshold` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isSetProposalThresholdSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isSetProposalThresholdSupported(["0x..."]);
 * ```
 */
export declare function isSetProposalThresholdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setProposalThreshold" function.
 * @param options - The options for the setProposalThreshold function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetProposalThresholdParams } from "thirdweb/extensions/vote";
 * const result = encodeSetProposalThresholdParams({
 *  newProposalThreshold: ...,
 * });
 * ```
 */
export declare function encodeSetProposalThresholdParams(options: SetProposalThresholdParams): `0x${string}`;
/**
 * Encodes the "setProposalThreshold" function into a Hex string with its parameters.
 * @param options - The options for the setProposalThreshold function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeSetProposalThreshold } from "thirdweb/extensions/vote";
 * const result = encodeSetProposalThreshold({
 *  newProposalThreshold: ...,
 * });
 * ```
 */
export declare function encodeSetProposalThreshold(options: SetProposalThresholdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setProposalThreshold" function on the contract.
 * @param options - The options for the "setProposalThreshold" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setProposalThreshold } from "thirdweb/extensions/vote";
 *
 * const transaction = setProposalThreshold({
 *  contract,
 *  newProposalThreshold: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setProposalThreshold(options: BaseTransactionOptions<SetProposalThresholdParams | {
    asyncParams: () => Promise<SetProposalThresholdParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setProposalThreshold.d.ts.map