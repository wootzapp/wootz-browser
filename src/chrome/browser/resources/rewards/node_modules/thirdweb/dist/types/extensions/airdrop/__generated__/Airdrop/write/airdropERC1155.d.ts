import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC1155" function.
 */
export type AirdropERC1155Params = WithOverrides<{
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_tokenAddress";
    }>;
    contents: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "_contents";
        components: [
            {
                type: "address";
                name: "recipient";
            },
            {
                type: "uint256";
                name: "tokenId";
            },
            {
                type: "uint256";
                name: "amount";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x2d89e38b";
/**
 * Checks if the `airdropERC1155` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC1155` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isAirdropERC1155Supported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isAirdropERC1155Supported(["0x..."]);
 * ```
 */
export declare function isAirdropERC1155Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC1155" function.
 * @param options - The options for the airdropERC1155 function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC1155Params } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC1155Params({
 *  tokenAddress: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC1155Params(options: AirdropERC1155Params): `0x${string}`;
/**
 * Encodes the "airdropERC1155" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC1155 function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC1155 } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC1155({
 *  tokenAddress: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC1155(options: AirdropERC1155Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC1155" function on the contract.
 * @param options - The options for the "airdropERC1155" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC1155 } from "thirdweb/extensions/airdrop";
 *
 * const transaction = airdropERC1155({
 *  contract,
 *  tokenAddress: ...,
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
export declare function airdropERC1155(options: BaseTransactionOptions<AirdropERC1155Params | {
    asyncParams: () => Promise<AirdropERC1155Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC1155.d.ts.map