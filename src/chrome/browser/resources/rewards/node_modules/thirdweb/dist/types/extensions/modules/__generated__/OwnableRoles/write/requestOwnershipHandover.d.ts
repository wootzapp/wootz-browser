import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0x25692962";
/**
 * Checks if the `requestOwnershipHandover` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `requestOwnershipHandover` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRequestOwnershipHandoverSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRequestOwnershipHandoverSupported(["0x..."]);
 * ```
 */
export declare function isRequestOwnershipHandoverSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "requestOwnershipHandover" function on the contract.
 * @param options - The options for the "requestOwnershipHandover" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { requestOwnershipHandover } from "thirdweb/extensions/modules";
 *
 * const transaction = requestOwnershipHandover();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function requestOwnershipHandover(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=requestOwnershipHandover.d.ts.map