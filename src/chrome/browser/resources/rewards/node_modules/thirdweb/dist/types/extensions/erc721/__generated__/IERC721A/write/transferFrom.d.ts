import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transferFrom" function.
 */
export type TransferFromParams = WithOverrides<{
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
export declare const FN_SELECTOR: "0x23b872dd";
/**
 * Checks if the `transferFrom` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transferFrom` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTransferFromSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isTransferFromSupported(["0x..."]);
 * ```
 */
export declare function isTransferFromSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transferFrom" function.
 * @param options - The options for the transferFrom function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTransferFromParams } from "thirdweb/extensions/erc721";
 * const result = encodeTransferFromParams({
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeTransferFromParams(options: TransferFromParams): `0x${string}`;
/**
 * Encodes the "transferFrom" function into a Hex string with its parameters.
 * @param options - The options for the transferFrom function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTransferFrom } from "thirdweb/extensions/erc721";
 * const result = encodeTransferFrom({
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeTransferFrom(options: TransferFromParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transferFrom" function on the contract.
 * @param options - The options for the "transferFrom" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transferFrom } from "thirdweb/extensions/erc721";
 *
 * const transaction = transferFrom({
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
export declare function transferFrom(options: BaseTransactionOptions<TransferFromParams | {
    asyncParams: () => Promise<TransferFromParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transferFrom.d.ts.map