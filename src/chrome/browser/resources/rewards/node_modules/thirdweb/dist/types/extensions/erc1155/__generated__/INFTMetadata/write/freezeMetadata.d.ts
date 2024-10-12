import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
export declare const FN_SELECTOR: "0xd111515d";
/**
 * Checks if the `freezeMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `freezeMetadata` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isFreezeMetadataSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isFreezeMetadataSupported(["0x..."]);
 * ```
 */
export declare function isFreezeMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Prepares a transaction to call the "freezeMetadata" function on the contract.
 * @param options - The options for the "freezeMetadata" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { freezeMetadata } from "thirdweb/extensions/erc1155";
 *
 * const transaction = freezeMetadata();
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function freezeMetadata(options: BaseTransactionOptions): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=freezeMetadata.d.ts.map