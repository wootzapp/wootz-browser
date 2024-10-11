import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "reveal" function.
 */
export type RevealParams = WithOverrides<{
    identifier: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "identifier";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
    }>;
}>;
export declare const FN_SELECTOR: "0xce805642";
/**
 * Checks if the `reveal` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `reveal` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isRevealSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isRevealSupported(["0x..."]);
 * ```
 */
export declare function isRevealSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "reveal" function.
 * @param options - The options for the reveal function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeRevealParams } from "thirdweb/extensions/erc721";
 * const result = encodeRevealParams({
 *  identifier: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeRevealParams(options: RevealParams): `0x${string}`;
/**
 * Encodes the "reveal" function into a Hex string with its parameters.
 * @param options - The options for the reveal function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeReveal } from "thirdweb/extensions/erc721";
 * const result = encodeReveal({
 *  identifier: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeReveal(options: RevealParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "reveal" function on the contract.
 * @param options - The options for the "reveal" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { reveal } from "thirdweb/extensions/erc721";
 *
 * const transaction = reveal({
 *  contract,
 *  identifier: ...,
 *  key: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function reveal(options: BaseTransactionOptions<RevealParams | {
    asyncParams: () => Promise<RevealParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=reveal.d.ts.map