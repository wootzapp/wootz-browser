import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "mintWithSignature" function.
 */
export type MintWithSignatureParams = WithOverrides<{
    payload: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "payload";
        components: [
            {
                type: "address";
                name: "to";
            },
            {
                type: "address";
                name: "royaltyRecipient";
            },
            {
                type: "uint256";
                name: "royaltyBps";
            },
            {
                type: "address";
                name: "primarySaleRecipient";
            },
            {
                type: "uint256";
                name: "tokenId";
            },
            {
                type: "string";
                name: "uri";
            },
            {
                type: "uint256";
                name: "quantity";
            },
            {
                type: "uint256";
                name: "pricePerToken";
            },
            {
                type: "address";
                name: "currency";
            },
            {
                type: "uint128";
                name: "validityStartTimestamp";
            },
            {
                type: "uint128";
                name: "validityEndTimestamp";
            },
            {
                type: "bytes32";
                name: "uid";
            }
        ];
    }>;
    signature: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "signature";
    }>;
}>;
export declare const FN_SELECTOR: "0x98a6e993";
/**
 * Checks if the `mintWithSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mintWithSignature` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isMintWithSignatureSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isMintWithSignatureSupported(["0x..."]);
 * ```
 */
export declare function isMintWithSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mintWithSignature" function.
 * @param options - The options for the mintWithSignature function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeMintWithSignatureParams } from "thirdweb/extensions/erc1155";
 * const result = encodeMintWithSignatureParams({
 *  payload: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeMintWithSignatureParams(options: MintWithSignatureParams): `0x${string}`;
/**
 * Encodes the "mintWithSignature" function into a Hex string with its parameters.
 * @param options - The options for the mintWithSignature function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeMintWithSignature } from "thirdweb/extensions/erc1155";
 * const result = encodeMintWithSignature({
 *  payload: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeMintWithSignature(options: MintWithSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "mintWithSignature" function on the contract.
 * @param options - The options for the "mintWithSignature" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { mintWithSignature } from "thirdweb/extensions/erc1155";
 *
 * const transaction = mintWithSignature({
 *  contract,
 *  payload: ...,
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