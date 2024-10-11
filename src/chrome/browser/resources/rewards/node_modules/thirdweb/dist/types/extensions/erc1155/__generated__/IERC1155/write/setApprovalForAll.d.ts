import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setApprovalForAll" function.
 */
export type SetApprovalForAllParams = WithOverrides<{
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_operator";
    }>;
    approved: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "_approved";
    }>;
}>;
export declare const FN_SELECTOR: "0xa22cb465";
/**
 * Checks if the `setApprovalForAll` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setApprovalForAll` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isSetApprovalForAllSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isSetApprovalForAllSupported(["0x..."]);
 * ```
 */
export declare function isSetApprovalForAllSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setApprovalForAll" function.
 * @param options - The options for the setApprovalForAll function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetApprovalForAllParams } from "thirdweb/extensions/erc1155";
 * const result = encodeSetApprovalForAllParams({
 *  operator: ...,
 *  approved: ...,
 * });
 * ```
 */
export declare function encodeSetApprovalForAllParams(options: SetApprovalForAllParams): `0x${string}`;
/**
 * Encodes the "setApprovalForAll" function into a Hex string with its parameters.
 * @param options - The options for the setApprovalForAll function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetApprovalForAll } from "thirdweb/extensions/erc1155";
 * const result = encodeSetApprovalForAll({
 *  operator: ...,
 *  approved: ...,
 * });
 * ```
 */
export declare function encodeSetApprovalForAll(options: SetApprovalForAllParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setApprovalForAll" function on the contract.
 * @param options - The options for the "setApprovalForAll" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setApprovalForAll } from "thirdweb/extensions/erc1155";
 *
 * const transaction = setApprovalForAll({
 *  contract,
 *  operator: ...,
 *  approved: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setApprovalForAll(options: BaseTransactionOptions<SetApprovalForAllParams | {
    asyncParams: () => Promise<SetApprovalForAllParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setApprovalForAll.d.ts.map