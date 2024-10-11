import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "lazyMint" function.
 */
export type LazyMintParams = WithOverrides<{
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amount";
    }>;
    baseURIForTokens: AbiParameterToPrimitiveType<{
        type: "string";
        name: "baseURIForTokens";
    }>;
    extraData: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "extraData";
    }>;
}>;
export declare const FN_SELECTOR: "0xd37c353b";
/**
 * Checks if the `lazyMint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `lazyMint` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isLazyMintSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isLazyMintSupported(["0x..."]);
 * ```
 */
export declare function isLazyMintSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "lazyMint" function.
 * @param options - The options for the lazyMint function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeLazyMintParams } from "thirdweb/extensions/erc1155";
 * const result = encodeLazyMintParams({
 *  amount: ...,
 *  baseURIForTokens: ...,
 *  extraData: ...,
 * });
 * ```
 */
export declare function encodeLazyMintParams(options: LazyMintParams): `0x${string}`;
/**
 * Encodes the "lazyMint" function into a Hex string with its parameters.
 * @param options - The options for the lazyMint function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeLazyMint } from "thirdweb/extensions/erc1155";
 * const result = encodeLazyMint({
 *  amount: ...,
 *  baseURIForTokens: ...,
 *  extraData: ...,
 * });
 * ```
 */
export declare function encodeLazyMint(options: LazyMintParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "lazyMint" function on the contract.
 * @param options - The options for the "lazyMint" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { lazyMint } from "thirdweb/extensions/erc1155";
 *
 * const transaction = lazyMint({
 *  contract,
 *  amount: ...,
 *  baseURIForTokens: ...,
 *  extraData: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function lazyMint(options: BaseTransactionOptions<LazyMintParams | {
    asyncParams: () => Promise<LazyMintParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=lazyMint.d.ts.map