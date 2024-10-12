import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transferOwnership" function.
 */
export type TransferOwnershipParams = WithOverrides<{
    newOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newOwner";
    }>;
}>;
export declare const FN_SELECTOR: "0xf2fde38b";
/**
 * Checks if the `transferOwnership` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transferOwnership` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isTransferOwnershipSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isTransferOwnershipSupported(["0x..."]);
 * ```
 */
export declare function isTransferOwnershipSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transferOwnership" function.
 * @param options - The options for the transferOwnership function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeTransferOwnershipParams } from "thirdweb/extensions/modules";
 * const result = encodeTransferOwnershipParams({
 *  newOwner: ...,
 * });
 * ```
 */
export declare function encodeTransferOwnershipParams(options: TransferOwnershipParams): `0x${string}`;
/**
 * Encodes the "transferOwnership" function into a Hex string with its parameters.
 * @param options - The options for the transferOwnership function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeTransferOwnership } from "thirdweb/extensions/modules";
 * const result = encodeTransferOwnership({
 *  newOwner: ...,
 * });
 * ```
 */
export declare function encodeTransferOwnership(options: TransferOwnershipParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transferOwnership" function on the contract.
 * @param options - The options for the "transferOwnership" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transferOwnership } from "thirdweb/extensions/modules";
 *
 * const transaction = transferOwnership({
 *  contract,
 *  newOwner: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function transferOwnership(options: BaseTransactionOptions<TransferOwnershipParams | {
    asyncParams: () => Promise<TransferOwnershipParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transferOwnership.d.ts.map