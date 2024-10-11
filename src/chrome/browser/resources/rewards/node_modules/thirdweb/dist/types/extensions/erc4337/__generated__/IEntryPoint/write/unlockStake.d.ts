import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0xbb9fe6bf";
/**
 * Checks if the `unlockStake` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `unlockStake` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isUnlockStakeSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isUnlockStakeSupported(["0x..."]);
 * ```
 */
export declare function isUnlockStakeSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "unlockStake" function on the contract.
 * @param options - The options for the "unlockStake" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { unlockStake } from "thirdweb/extensions/erc4337";
 *
 * const transaction = unlockStake();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function unlockStake(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=unlockStake.d.ts.map