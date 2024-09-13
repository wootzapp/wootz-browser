import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0x372500ab";
/**
 * Checks if the `claimRewards` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimRewards` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isClaimRewardsSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isClaimRewardsSupported(["0x..."]);
 * ```
 */
export declare function isClaimRewardsSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "claimRewards" function on the contract.
 * @param options - The options for the "claimRewards" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimRewards } from "thirdweb/extensions/erc20";
 *
 * const transaction = claimRewards();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function claimRewards(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimRewards.d.ts.map