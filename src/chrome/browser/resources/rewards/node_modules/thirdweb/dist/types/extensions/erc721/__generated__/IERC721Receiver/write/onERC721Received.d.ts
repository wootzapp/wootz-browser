import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "onERC721Received" function.
 */
export type OnERC721ReceivedParams = WithOverrides<{
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
    }>;
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x150b7a02";
/**
 * Checks if the `onERC721Received` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `onERC721Received` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isOnERC721ReceivedSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isOnERC721ReceivedSupported(["0x..."]);
 * ```
 */
export declare function isOnERC721ReceivedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "onERC721Received" function.
 * @param options - The options for the onERC721Received function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeOnERC721ReceivedParams } from "thirdweb/extensions/erc721";
 * const result = encodeOnERC721ReceivedParams({
 *  operator: ...,
 *  from: ...,
 *  tokenId: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC721ReceivedParams(options: OnERC721ReceivedParams): `0x${string}`;
/**
 * Encodes the "onERC721Received" function into a Hex string with its parameters.
 * @param options - The options for the onERC721Received function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeOnERC721Received } from "thirdweb/extensions/erc721";
 * const result = encodeOnERC721Received({
 *  operator: ...,
 *  from: ...,
 *  tokenId: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC721Received(options: OnERC721ReceivedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "onERC721Received" function on the contract.
 * @param options - The options for the "onERC721Received" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { onERC721Received } from "thirdweb/extensions/erc721";
 *
 * const transaction = onERC721Received({
 *  contract,
 *  operator: ...,
 *  from: ...,
 *  tokenId: ...,
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
export declare function onERC721Received(options: BaseTransactionOptions<OnERC721ReceivedParams | {
    asyncParams: () => Promise<OnERC721ReceivedParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=onERC721Received.d.ts.map