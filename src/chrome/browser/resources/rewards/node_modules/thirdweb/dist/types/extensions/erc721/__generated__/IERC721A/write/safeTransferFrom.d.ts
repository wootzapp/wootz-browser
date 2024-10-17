import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "safeTransferFrom" function.
 */
export type SafeTransferFromParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
}>;
export declare const FN_SELECTOR: "0x42842e0e";
/**
 * Checks if the `safeTransferFrom` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `safeTransferFrom` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isSafeTransferFromSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isSafeTransferFromSupported(["0x..."]);
 * ```
 */
export declare function isSafeTransferFromSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "safeTransferFrom" function.
 * @param options - The options for the safeTransferFrom function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSafeTransferFromParams } from "thirdweb/extensions/erc721";
 * const result = encodeSafeTransferFromParams({
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeSafeTransferFromParams(options: SafeTransferFromParams): `0x${string}`;
/**
 * Encodes the "safeTransferFrom" function into a Hex string with its parameters.
 * @param options - The options for the safeTransferFrom function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeSafeTransferFrom } from "thirdweb/extensions/erc721";
 * const result = encodeSafeTransferFrom({
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeSafeTransferFrom(options: SafeTransferFromParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "safeTransferFrom" function on the contract.
 * @param options - The options for the "safeTransferFrom" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { safeTransferFrom } from "thirdweb/extensions/erc721";
 *
 * const transaction = safeTransferFrom({
 *  contract,
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function safeTransferFrom(options: BaseTransactionOptions<SafeTransferFromParams | {
    asyncParams: () => Promise<SafeTransferFromParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=safeTransferFrom.d.ts.map