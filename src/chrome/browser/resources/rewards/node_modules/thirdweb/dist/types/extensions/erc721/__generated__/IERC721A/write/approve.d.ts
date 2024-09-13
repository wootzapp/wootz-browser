import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "approve" function.
 */
export type ApproveParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
}>;
export declare const FN_SELECTOR: "0x095ea7b3";
/**
 * Checks if the `approve` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `approve` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isApproveSupported } from "thirdweb/extensions/erc721";
 *
 * const supported = isApproveSupported(["0x..."]);
 * ```
 */
export declare function isApproveSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "approve" function.
 * @param options - The options for the approve function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeApproveParams } from "thirdweb/extensions/erc721";
 * const result = encodeApproveParams({
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeApproveParams(options: ApproveParams): `0x${string}`;
/**
 * Encodes the "approve" function into a Hex string with its parameters.
 * @param options - The options for the approve function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeApprove } from "thirdweb/extensions/erc721";
 * const result = encodeApprove({
 *  to: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeApprove(options: ApproveParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "approve" function on the contract.
 * @param options - The options for the "approve" function.
 * @returns A prepared transaction object.
 * @extension ERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { approve } from "thirdweb/extensions/erc721";
 *
 * const transaction = approve({
 *  contract,
 *  to: ...,
 *  tokenId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function approve(options: BaseTransactionOptions<ApproveParams | {
    asyncParams: () => Promise<ApproveParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=approve.d.ts.map