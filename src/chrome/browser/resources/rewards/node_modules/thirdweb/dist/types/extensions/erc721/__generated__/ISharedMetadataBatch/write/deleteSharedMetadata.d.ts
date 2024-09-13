import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "deleteSharedMetadata" function.
 */
export type DeleteSharedMetadataParams = WithOverrides<{
    id: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "id";
    }>;
}>;
export declare const FN_SELECTOR: "0x1ebb2422";
/**
 * Checks if the `deleteSharedMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `deleteSharedMetadata` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isDeleteSharedMetadataSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isDeleteSharedMetadataSupported(["0x..."]);
 * ```
 */
export declare function isDeleteSharedMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "deleteSharedMetadata" function.
 * @param options - The options for the deleteSharedMetadata function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeDeleteSharedMetadataParams } from "thirdweb/extensions/erc721";
 * const result = encodeDeleteSharedMetadataParams({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeDeleteSharedMetadataParams(options: DeleteSharedMetadataParams): `0x${string}`;
/**
 * Encodes the "deleteSharedMetadata" function into a Hex string with its parameters.
 * @param options - The options for the deleteSharedMetadata function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeDeleteSharedMetadata } from "thirdweb/extensions/erc721";
 * const result = encodeDeleteSharedMetadata({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeDeleteSharedMetadata(options: DeleteSharedMetadataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "deleteSharedMetadata" function on the contract.
 * @param options - The options for the "deleteSharedMetadata" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { deleteSharedMetadata } from "thirdweb/extensions/erc721";
 *
 * const transaction = deleteSharedMetadata({
 *  contract,
 *  id: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function deleteSharedMetadata(options: BaseTransactionOptions<DeleteSharedMetadataParams | {
    asyncParams: () => Promise<DeleteSharedMetadataParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=deleteSharedMetadata.d.ts.map