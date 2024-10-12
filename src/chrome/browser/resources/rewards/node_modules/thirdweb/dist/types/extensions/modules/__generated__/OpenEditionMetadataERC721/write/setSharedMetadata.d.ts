import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setSharedMetadata" function.
 */
export type SetSharedMetadataParams = WithOverrides<{
    metadata: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "_metadata";
        components: [
            {
                type: "string";
                name: "name";
            },
            {
                type: "string";
                name: "description";
            },
            {
                type: "string";
                name: "imageURI";
            },
            {
                type: "string";
                name: "animationURI";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0xa7d27d9d";
/**
 * Checks if the `setSharedMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setSharedMetadata` method is supported.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 *
 * const supported = OpenEditionMetadataERC721.isSetSharedMetadataSupported(["0x..."]);
 * ```
 */
export declare function isSetSharedMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setSharedMetadata" function.
 * @param options - The options for the setSharedMetadata function.
 * @returns The encoded ABI parameters.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const result = OpenEditionMetadataERC721.encodeSetSharedMetadataParams({
 *  metadata: ...,
 * });
 * ```
 */
export declare function encodeSetSharedMetadataParams(options: SetSharedMetadataParams): `0x${string}`;
/**
 * Encodes the "setSharedMetadata" function into a Hex string with its parameters.
 * @param options - The options for the setSharedMetadata function.
 * @returns The encoded hexadecimal string.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const result = OpenEditionMetadataERC721.encodeSetSharedMetadata({
 *  metadata: ...,
 * });
 * ```
 */
export declare function encodeSetSharedMetadata(options: SetSharedMetadataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setSharedMetadata" function on the contract.
 * @param options - The options for the "setSharedMetadata" function.
 * @returns A prepared transaction object.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 *
 * const transaction = OpenEditionMetadataERC721.setSharedMetadata({
 *  contract,
 *  metadata: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setSharedMetadata(options: BaseTransactionOptions<SetSharedMetadataParams | {
    asyncParams: () => Promise<SetSharedMetadataParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setSharedMetadata.d.ts.map