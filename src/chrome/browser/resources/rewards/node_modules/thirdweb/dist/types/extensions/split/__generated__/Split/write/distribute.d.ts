import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0xe4fc6b6d";
/**
 * Checks if the `distribute` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `distribute` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isDistributeSupported } from "thirdweb/extensions/split";
 *
 * const supported = isDistributeSupported(["0x..."]);
 * ```
 */
export declare function isDistributeSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "distribute" function on the contract.
 * @param options - The options for the "distribute" function.
 * @returns A prepared transaction object.
 * @extension SPLIT
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { distribute } from "thirdweb/extensions/split";
 *
 * const transaction = distribute();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function distribute(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=distribute.d.ts.map