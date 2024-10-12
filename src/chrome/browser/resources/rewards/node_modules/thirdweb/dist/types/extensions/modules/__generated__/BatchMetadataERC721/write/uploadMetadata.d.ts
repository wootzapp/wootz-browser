import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "uploadMetadata" function.
 */
export type UploadMetadataParams = WithOverrides<{
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_amount";
    }>;
    baseURI: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_baseURI";
    }>;
}>;
export declare const FN_SELECTOR: "0xbfa2f36e";
/**
 * Checks if the `uploadMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `uploadMetadata` method is supported.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 *
 * const supported = BatchMetadataERC721.isUploadMetadataSupported(["0x..."]);
 * ```
 */
export declare function isUploadMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "uploadMetadata" function.
 * @param options - The options for the uploadMetadata function.
 * @returns The encoded ABI parameters.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 * const result = BatchMetadataERC721.encodeUploadMetadataParams({
 *  amount: ...,
 *  baseURI: ...,
 * });
 * ```
 */
export declare function encodeUploadMetadataParams(options: UploadMetadataParams): `0x${string}`;
/**
 * Encodes the "uploadMetadata" function into a Hex string with its parameters.
 * @param options - The options for the uploadMetadata function.
 * @returns The encoded hexadecimal string.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 * const result = BatchMetadataERC721.encodeUploadMetadata({
 *  amount: ...,
 *  baseURI: ...,
 * });
 * ```
 */
export declare function encodeUploadMetadata(options: UploadMetadataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "uploadMetadata" function on the contract.
 * @param options - The options for the "uploadMetadata" function.
 * @returns A prepared transaction object.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 *
 * const transaction = BatchMetadataERC721.uploadMetadata({
 *  contract,
 *  amount: ...,
 *  baseURI: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function uploadMetadata(options: BaseTransactionOptions<UploadMetadataParams | {
    asyncParams: () => Promise<UploadMetadataParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=uploadMetadata.d.ts.map