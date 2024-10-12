import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claimERC20" function.
 */
export type ClaimERC20Params = WithOverrides<{
    token: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_token";
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_receiver";
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
export declare const FN_SELECTOR: "0xecf3d3d4";
/**
 * Checks if the `claimERC20` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimERC20` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isClaimERC20Supported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isClaimERC20Supported(["0x..."]);
 * ```
 */
export declare function isClaimERC20Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claimERC20" function.
 * @param options - The options for the claimERC20 function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC20Params } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC20Params({
 *  token: ...,
 *  receiver: ...,
 *  quantity: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC20Params(options: ClaimERC20Params): `0x${string}`;
/**
 * Encodes the "claimERC20" function into a Hex string with its parameters.
 * @param options - The options for the claimERC20 function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeClaimERC20 } from "thirdweb/extensions/airdrop";
 * const result = encodeClaimERC20({
 *  token: ...,
 *  receiver: ...,
 *  quantity: ...,
 *  proofs: ...,
 * });
 * ```
 */
export declare function encodeClaimERC20(options: ClaimERC20Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claimERC20" function on the contract.
 * @param options - The options for the "claimERC20" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claimERC20 } from "thirdweb/extensions/airdrop";
 *
 * const transaction = claimERC20({
 *  contract,
 *  token: ...,
 *  receiver: ...,
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
export declare function claimERC20(options: BaseTransactionOptions<ClaimERC20Params | {
    asyncParams: () => Promise<ClaimERC20Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claimERC20.d.ts.map