import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0xd0e30db0";
/**
 * Checks if the `deposit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `deposit` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isDepositSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isDepositSupported(["0x..."]);
 * ```
 */
export declare function isDepositSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "deposit" function on the contract.
 * @param options - The options for the "deposit" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { deposit } from "thirdweb/extensions/erc20";
 *
 * const transaction = deposit();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function deposit(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=deposit.d.ts.map