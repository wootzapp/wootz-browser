import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "completeOwnershipHandover" function.
 */
export type CompleteOwnershipHandoverParams = WithOverrides<{
    pendingOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "pendingOwner";
    }>;
}>;
export declare const FN_SELECTOR: "0xf04e283e";
/**
 * Checks if the `completeOwnershipHandover` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `completeOwnershipHandover` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isCompleteOwnershipHandoverSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isCompleteOwnershipHandoverSupported(["0x..."]);
 * ```
 */
export declare function isCompleteOwnershipHandoverSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "completeOwnershipHandover" function.
 * @param options - The options for the completeOwnershipHandover function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeCompleteOwnershipHandoverParams } from "thirdweb/extensions/modules";
 * const result = encodeCompleteOwnershipHandoverParams({
 *  pendingOwner: ...,
 * });
 * ```
 */
export declare function encodeCompleteOwnershipHandoverParams(options: CompleteOwnershipHandoverParams): `0x${string}`;
/**
 * Encodes the "completeOwnershipHandover" function into a Hex string with its parameters.
 * @param options - The options for the completeOwnershipHandover function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeCompleteOwnershipHandover } from "thirdweb/extensions/modules";
 * const result = encodeCompleteOwnershipHandover({
 *  pendingOwner: ...,
 * });
 * ```
 */
export declare function encodeCompleteOwnershipHandover(options: CompleteOwnershipHandoverParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "completeOwnershipHandover" function on the contract.
 * @param options - The options for the "completeOwnershipHandover" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { completeOwnershipHandover } from "thirdweb/extensions/modules";
 *
 * const transaction = completeOwnershipHandover({
 *  contract,
 *  pendingOwner: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function completeOwnershipHandover(options: BaseTransactionOptions<CompleteOwnershipHandoverParams | {
    asyncParams: () => Promise<CompleteOwnershipHandoverParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=completeOwnershipHandover.d.ts.map