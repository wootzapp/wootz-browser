import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setSharedMetadata" function.
 */
export type SetSharedMetadataParams = WithOverrides<{
    metadata: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "metadata";
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
    id: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "id";
    }>;
}>;
export declare const FN_SELECTOR: "0x696b0c1a";
/**
 * Checks if the `setSharedMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setSharedMetadata` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isSetSharedMetadataSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isSetSharedMetadataSupported(["0x..."]);
 * ```
 */
export declare function isSetSharedMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setSharedMetadata" function.
 * @param options - The options for the setSharedMetadata function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSetSharedMetadataParams } from "thirdweb/extensions/erc721";
 * const result = encodeSetSharedMetadataParams({
 *  metadata: ...,
 *  id: ...,
 * });
 * ```
 */
export declare function encodeSetSharedMetadataParams(options: SetSharedMetadataParams): `0x${string}`;
/**
 * Encodes the "setSharedMetadata" function into a Hex string with its parameters.
 * @param options - The options for the setSharedMetadata function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSetSharedMetadata } from "thirdweb/extensions/erc721";
 * const result = encodeSetSharedMetadata({
 *  metadata: ...,
 *  id: ...,
 * });
 * ```
 */
export declare function encodeSetSharedMetadata(options: SetSharedMetadataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setSharedMetadata" function on the contract.
 * @param options - The options for the "setSharedMetadata" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setSharedMetadata } from "thirdweb/extensions/erc721";
 *
 * const transaction = setSharedMetadata({
 *  contract,
 *  metadata: ...,
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
export declare function setSharedMetadata(options: BaseTransactionOptions<SetSharedMetadataParams | {
    asyncParams: () => Promise<SetSharedMetadataParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setSharedMetadata.d.ts.map