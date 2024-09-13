import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "safeBatchTransferFrom" function.
 */
export type SafeBatchTransferFromParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_to";
    }>;
    tokenIds: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "tokenIds";
    }>;
    values: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "_values";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "_data";
    }>;
}>;
export declare const FN_SELECTOR: "0x2eb2c2d6";
/**
 * Checks if the `safeBatchTransferFrom` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `safeBatchTransferFrom` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isSafeBatchTransferFromSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isSafeBatchTransferFromSupported(["0x..."]);
 * ```
 */
export declare function isSafeBatchTransferFromSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "safeBatchTransferFrom" function.
 * @param options - The options for the safeBatchTransferFrom function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSafeBatchTransferFromParams } from "thirdweb/extensions/erc1155";
 * const result = encodeSafeBatchTransferFromParams({
 *  from: ...,
 *  to: ...,
 *  tokenIds: ...,
 *  values: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeSafeBatchTransferFromParams(options: SafeBatchTransferFromParams): `0x${string}`;
/**
 * Encodes the "safeBatchTransferFrom" function into a Hex string with its parameters.
 * @param options - The options for the safeBatchTransferFrom function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSafeBatchTransferFrom } from "thirdweb/extensions/erc1155";
 * const result = encodeSafeBatchTransferFrom({
 *  from: ...,
 *  to: ...,
 *  tokenIds: ...,
 *  values: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeSafeBatchTransferFrom(options: SafeBatchTransferFromParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "safeBatchTransferFrom" function on the contract.
 * @param options - The options for the "safeBatchTransferFrom" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { safeBatchTransferFrom } from "thirdweb/extensions/erc1155";
 *
 * const transaction = safeBatchTransferFrom({
 *  contract,
 *  from: ...,
 *  to: ...,
 *  tokenIds: ...,
 *  values: ...,
 *  data: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function safeBatchTransferFrom(options: BaseTransactionOptions<SafeBatchTransferFromParams | {
    asyncParams: () => Promise<SafeBatchTransferFromParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=safeBatchTransferFrom.d.ts.map