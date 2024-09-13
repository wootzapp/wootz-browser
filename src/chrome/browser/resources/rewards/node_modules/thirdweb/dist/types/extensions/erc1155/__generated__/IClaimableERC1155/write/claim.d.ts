import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claim" function.
 */
export type ClaimParams = WithOverrides<{
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
}>;
export declare const FN_SELECTOR: "0x2bc43fd9";
/**
 * Checks if the `claim` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claim` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isClaimSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isClaimSupported(["0x..."]);
 * ```
 */
export declare function isClaimSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claim" function.
 * @param options - The options for the claim function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaimParams } from "thirdweb/extensions/erc1155";
 * const result = encodeClaimParams({
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 * });
 * ```
 */
export declare function encodeClaimParams(options: ClaimParams): `0x${string}`;
/**
 * Encodes the "claim" function into a Hex string with its parameters.
 * @param options - The options for the claim function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeClaim } from "thirdweb/extensions/erc1155";
 * const result = encodeClaim({
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 * });
 * ```
 */
export declare function encodeClaim(options: ClaimParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claim" function on the contract.
 * @param options - The options for the "claim" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claim } from "thirdweb/extensions/erc1155";
 *
 * const transaction = claim({
 *  contract,
 *  receiver: ...,
 *  tokenId: ...,
 *  quantity: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function claim(options: BaseTransactionOptions<ClaimParams | {
    asyncParams: () => Promise<ClaimParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claim.d.ts.map