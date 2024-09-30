import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "mintWithSignature" function.
 */
export type MintWithSignatureParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amount";
    }>;
    baseURI: AbiParameterToPrimitiveType<{
        type: "string";
        name: "baseURI";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
    signature: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "signature";
    }>;
}>;
export declare const FN_SELECTOR: "0xe6bd6ada";
/**
 * Checks if the `mintWithSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mintWithSignature` method is supported.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 *
 * const supported = ERC1155Core.isMintWithSignatureSupported(["0x..."]);
 * ```
 */
export declare function isMintWithSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mintWithSignature" function.
 * @param options - The options for the mintWithSignature function.
 * @returns The encoded ABI parameters.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 * const result = ERC1155Core.encodeMintWithSignatureParams({
 *  to: ...,
 *  tokenId: ...,
 *  amount: ...,
 *  baseURI: ...,
 *  data: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeMintWithSignatureParams(options: MintWithSignatureParams): `0x${string}`;
/**
 * Encodes the "mintWithSignature" function into a Hex string with its parameters.
 * @param options - The options for the mintWithSignature function.
 * @returns The encoded hexadecimal string.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 * const result = ERC1155Core.encodeMintWithSignature({
 *  to: ...,
 *  tokenId: ...,
 *  amount: ...,
 *  baseURI: ...,
 *  data: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeMintWithSignature(options: MintWithSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "mintWithSignature" function on the contract.
 * @param options - The options for the "mintWithSignature" function.
 * @returns A prepared transaction object.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ERC1155Core } from "thirdweb/modules";
 *
 * const transaction = ERC1155Core.mintWithSignature({
 *  contract,
 *  to: ...,
 *  tokenId: ...,
 *  amount: ...,
 *  baseURI: ...,
 *  data: ...,
 *  signature: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function mintWithSignature(options: BaseTransactionOptions<MintWithSignatureParams | {
    asyncParams: () => Promise<MintWithSignatureParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mintWithSignature.d.ts.map