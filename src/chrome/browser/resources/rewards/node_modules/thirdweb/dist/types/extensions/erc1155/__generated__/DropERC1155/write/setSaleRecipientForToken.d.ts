import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setSaleRecipientForToken" function.
 */
export type SetSaleRecipientForTokenParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
    saleRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_saleRecipient";
    }>;
}>;
export declare const FN_SELECTOR: "0x29c49b9b";
/**
 * Checks if the `setSaleRecipientForToken` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setSaleRecipientForToken` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isSetSaleRecipientForTokenSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isSetSaleRecipientForTokenSupported(["0x..."]);
 * ```
 */
export declare function isSetSaleRecipientForTokenSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setSaleRecipientForToken" function.
 * @param options - The options for the setSaleRecipientForToken function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetSaleRecipientForTokenParams } from "thirdweb/extensions/erc1155";
 * const result = encodeSetSaleRecipientForTokenParams({
 *  tokenId: ...,
 *  saleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetSaleRecipientForTokenParams(options: SetSaleRecipientForTokenParams): `0x${string}`;
/**
 * Encodes the "setSaleRecipientForToken" function into a Hex string with its parameters.
 * @param options - The options for the setSaleRecipientForToken function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetSaleRecipientForToken } from "thirdweb/extensions/erc1155";
 * const result = encodeSetSaleRecipientForToken({
 *  tokenId: ...,
 *  saleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetSaleRecipientForToken(options: SetSaleRecipientForTokenParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setSaleRecipientForToken" function on the contract.
 * @param options - The options for the "setSaleRecipientForToken" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setSaleRecipientForToken } from "thirdweb/extensions/erc1155";
 *
 * const transaction = setSaleRecipientForToken({
 *  contract,
 *  tokenId: ...,
 *  saleRecipient: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setSaleRecipientForToken(options: BaseTransactionOptions<SetSaleRecipientForTokenParams | {
    asyncParams: () => Promise<SetSaleRecipientForTokenParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setSaleRecipientForToken.d.ts.map