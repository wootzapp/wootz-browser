import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claimERC1155" function.
 */
export type ClaimERC1155Params = WithOverrides<{
    token: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_token";
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_receiver";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
    quantity: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_quantity";
    }>;
    proofs: AbiParameterToPrimitiveType<{
        type: "bytes32[]";
        name: "_proofs";
    }>;
}>;
export declare const FN_SELECTOR: "0xc6fa26ab";
/**
 * Checks if the `claimERC1155` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimERC1155` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isClaimERC1155Supported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isClaimERC1155Supported(["0x..."]);
 * ```
 */
export declare function isClaimERC1155Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claimERC1155" function.
 * @param options - The options for the claimERC1155 function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC1155Params } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC1155Params({
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC1155Params(options: ClaimERC1155Params): `0x${string}`;
/**
 * Encodes the "claimERC1155" function into a Hex string with its parameters.
 * @param options - The options for the claimERC1155 function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC1155 } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC1155({
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC1155(options: ClaimERC1155Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claimERC1155" function on the contract.
 * @param options - The options for the "claimERC1155" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimERC1155 } from "thirdweb/extensions/airdrop";
 *
 * const transaction = claimERC1155({
 *  contract,
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 *  proofs: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function claimERC1155(options: BaseTransactionOptions<ClaimERC1155Params | {
    asyncParams: () => Promise<ClaimERC1155Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimERC1155.d.ts.map