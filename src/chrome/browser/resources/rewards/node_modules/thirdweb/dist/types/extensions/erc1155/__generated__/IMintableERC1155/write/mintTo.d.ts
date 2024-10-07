import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "mintTo" function.
 */
export type MintToParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "uri";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amount";
    }>;
}>;
export declare const FN_SELECTOR: "0xb03f4528";
/**
 * Checks if the `mintTo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mintTo` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isMintToSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isMintToSupported(["0x..."]);
 * ```
 */
export declare function isMintToSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mintTo" function.
 * @param options - The options for the mintTo function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeMintToParams } from "thirdweb/extensions/erc1155";
 * const result = encodeMintToParams({
 *  to: ...,
 *  tokenId: ...,
 *  uri: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeMintToParams(options: MintToParams): `0x${string}`;
/**
 * Encodes the "mintTo" function into a Hex string with its parameters.
 * @param options - The options for the mintTo function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeMintTo } from "thirdweb/extensions/erc1155";
 * const result = encodeMintTo({
 *  to: ...,
 *  tokenId: ...,
 *  uri: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeMintTo(options: MintToParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "mintTo" function on the contract.
 * @param options - The options for the "mintTo" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { mintTo } from "thirdweb/extensions/erc1155";
 *
 * const transaction = mintTo({
 *  contract,
 *  to: ...,
 *  tokenId: ...,
 *  uri: ...,
 *  amount: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function mintTo(options: BaseTransactionOptions<MintToParams | {
    asyncParams: () => Promise<MintToParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mintTo.d.ts.map