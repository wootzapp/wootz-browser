import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setTokenURI" function.
 */
export type SetTokenURIParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_uri";
    }>;
}>;
export declare const FN_SELECTOR: "0x162094c4";
/**
 * Checks if the `setTokenURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setTokenURI` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isSetTokenURISupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isSetTokenURISupported(["0x..."]);
 * ```
 */
export declare function isSetTokenURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setTokenURI" function.
 * @param options - The options for the setTokenURI function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSetTokenURIParams } from "thirdweb/extensions/erc721";
 * const result = encodeSetTokenURIParams({
 *  tokenId: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetTokenURIParams(options: SetTokenURIParams): `0x${string}`;
/**
 * Encodes the "setTokenURI" function into a Hex string with its parameters.
 * @param options - The options for the setTokenURI function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSetTokenURI } from "thirdweb/extensions/erc721";
 * const result = encodeSetTokenURI({
 *  tokenId: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetTokenURI(options: SetTokenURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setTokenURI" function on the contract.
 * @param options - The options for the "setTokenURI" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setTokenURI } from "thirdweb/extensions/erc721";
 *
 * const transaction = setTokenURI({
 *  contract,
 *  tokenId: ...,
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
export declare function setTokenURI(options: BaseTransactionOptions<SetTokenURIParams | {
    asyncParams: () => Promise<SetTokenURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setTokenURI.d.ts.map