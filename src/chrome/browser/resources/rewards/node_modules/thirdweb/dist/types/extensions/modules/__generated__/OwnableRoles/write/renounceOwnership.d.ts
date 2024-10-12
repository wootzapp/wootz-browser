import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0x715018a6";
/**
 * Checks if the `renounceOwnership` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `renounceOwnership` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRenounceOwnershipSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRenounceOwnershipSupported(["0x..."]);
 * ```
 */
export declare function isRenounceOwnershipSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "renounceOwnership" function on the contract.
 * @param options - The options for the "renounceOwnership" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { renounceOwnership } from "thirdweb/extensions/modules";
 *
 * const transaction = renounceOwnership();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function renounceOwnership(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=renounceOwnership.d.ts.map