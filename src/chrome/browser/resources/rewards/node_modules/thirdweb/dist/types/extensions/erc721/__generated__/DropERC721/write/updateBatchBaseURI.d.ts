import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "updateBatchBaseURI" function.
 */
export type UpdateBatchBaseURIParams = WithOverrides<{
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_index";
    }>;
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_uri";
    }>;
}>;
export declare const FN_SELECTOR: "0xde903ddd";
/**
 * Checks if the `updateBatchBaseURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `updateBatchBaseURI` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isUpdateBatchBaseURISupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isUpdateBatchBaseURISupported(["0x..."]);
 * ```
 */
export declare function isUpdateBatchBaseURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "updateBatchBaseURI" function.
 * @param options - The options for the updateBatchBaseURI function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeUpdateBatchBaseURIParams } from "thirdweb/extensions/erc721";
 * const result = encodeUpdateBatchBaseURIParams({
 *  index: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeUpdateBatchBaseURIParams(options: UpdateBatchBaseURIParams): `0x${string}`;
/**
 * Encodes the "updateBatchBaseURI" function into a Hex string with its parameters.
 * @param options - The options for the updateBatchBaseURI function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeUpdateBatchBaseURI } from "thirdweb/extensions/erc721";
 * const result = encodeUpdateBatchBaseURI({
 *  index: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeUpdateBatchBaseURI(options: UpdateBatchBaseURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "updateBatchBaseURI" function on the contract.
 * @param options - The options for the "updateBatchBaseURI" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { updateBatchBaseURI } from "thirdweb/extensions/erc721";
 *
 * const transaction = updateBatchBaseURI({
 *  contract,
 *  index: ...,
 *  uri: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function updateBatchBaseURI(options: BaseTransactionOptions<UpdateBatchBaseURIParams | {
    asyncParams: () => Promise<UpdateBatchBaseURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=updateBatchBaseURI.d.ts.map