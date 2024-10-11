import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
/**
 * @extension ERC4337
 */
export type RemoveSessionKeyOptions = {
    /**
     * The account that will perform the operation.
     */
    account: Account;
    /**
     * The address to remove as a session key.
     */
    sessionKeyAddress: string;
};
/**
 * Removes session key permissions for a specified address.
 * @param options - The options for the removeSessionKey function.
 * @returns The transaction object to be sent.
 * @example
 * ```ts
 * import { removeSessionKey } from 'thirdweb/extensions/erc4337';
 * import { sendTransaction } from 'thirdweb';
 *
 * const transaction = removeSessionKey({
 * contract,
 * account,
 * sessionKeyAddress
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 * @extension ERC4337
 */
export declare function removeSessionKey(options: BaseTransactionOptions<RemoveSessionKeyOptions>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
/**
 * Checks if the `isRemoveSessionKeySupported` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isRemoveSessionKeySupported` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isRemoveSessionKeySupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isRemoveSessionKeySupported(["0x..."]);
 * ```
 */
export declare function isRemoveSessionKeySupported(availableSelectors: string[]): boolean;
//# sourceMappingURL=removeSessionKey.d.ts.map