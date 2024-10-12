import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claimERC721" function.
 */
export type ClaimERC721Params = WithOverrides<{
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
    proofs: AbiParameterToPrimitiveType<{
        type: "bytes32[]";
        name: "_proofs";
    }>;
}>;
export declare const FN_SELECTOR: "0x1290be10";
/**
 * Checks if the `claimERC721` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimERC721` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isClaimERC721Supported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isClaimERC721Supported(["0x..."]);
 * ```
 */
export declare function isClaimERC721Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claimERC721" function.
 * @param options - The options for the claimERC721 function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC721Params } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC721Params({
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC721Params(options: ClaimERC721Params): `0x${string}`;
/**
 * Encodes the "claimERC721" function into a Hex string with its parameters.
 * @param options - The options for the claimERC721 function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC721 } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC721({
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC721(options: ClaimERC721Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claimERC721" function on the contract.
 * @param options - The options for the "claimERC721" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimERC721 } from "thirdweb/extensions/airdrop";
 *
 * const transaction = claimERC721({
 *  contract,
 *  token: ...,
 *  receiver: ...,
 *  tokenId: ...,
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
export declare function claimERC721(options: BaseTransactionOptions<ClaimERC721Params | {
    asyncParams: () => Promise<ClaimERC721Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimERC721.d.ts.map