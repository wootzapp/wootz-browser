import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
/**
 * @extension ERC4337
 */
export type RemoveAdminOptions = {
    /**
     * The admin account that will perform the operation.
     */
    account: Account;
    /**
     * The address to remove as an admin.
     */
    adminAddress: string;
};
/**
 * Removes admin permissions for a specified address.
 * @param options - The options for the removeAdmin function.
 * @returns The transaction object to be sent.
 * @example
 * ```ts
 * import { removeAdmin } from 'thirdweb/extensions/erc4337';
 * import { sendTransaction } from 'thirdweb';
 *
 * const transaction = removeAdmin({
 *  contract,
 *  account,
 *  adminAddress: '0x...'
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 * @extension ERC4337
 */
export declare function removeAdmin(options: BaseTransactionOptions<RemoveAdminOptions>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
/**
 * Checks if the `isRemoveAdminSupported` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isRemoveAdminSupported` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isRemoveAdminSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isRemoveAdminSupported(["0x..."]);
 * ```
 */
export declare function isRemoveAdminSupported(availableSelectors: string[]): boolean;
//# sourceMappingURL=removeAdmin.d.ts.map