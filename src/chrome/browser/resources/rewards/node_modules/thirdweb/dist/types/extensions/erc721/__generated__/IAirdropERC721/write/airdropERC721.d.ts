import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC721" function.
 */
export type AirdropERC721Params = WithOverrides<{
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
    }>;
    tokenOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenOwner";
    }>;
    contents: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "contents";
        components: [
            {
                type: "address";
                name: "recipient";
            },
            {
                type: "uint256";
                name: "tokenId";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x7c2c059d";
/**
 * Checks if the `airdropERC721` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC721` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isAirdropERC721Supported } from "thirdweb/extensions/erc721";
 *
 * const supported = isAirdropERC721Supported(["0x..."]);
 * ```
 */
export declare function isAirdropERC721Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC721" function.
 * @param options - The options for the airdropERC721 function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeAirdropERC721Params } from "thirdweb/extensions/erc721";
 * const result = encodeAirdropERC721Params({
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC721Params(options: AirdropERC721Params): `0x${string}`;
/**
 * Encodes the "airdropERC721" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC721 function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeAirdropERC721 } from "thirdweb/extensions/erc721";
 * const result = encodeAirdropERC721({
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC721(options: AirdropERC721Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC721" function on the contract.
 * @param options - The options for the "airdropERC721" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC721 } from "thirdweb/extensions/erc721";
 *
 * const transaction = airdropERC721({
 *  contract,
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function airdropERC721(options: BaseTransactionOptions<AirdropERC721Params | {
    asyncParams: () => Promise<AirdropERC721Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC721.d.ts.map