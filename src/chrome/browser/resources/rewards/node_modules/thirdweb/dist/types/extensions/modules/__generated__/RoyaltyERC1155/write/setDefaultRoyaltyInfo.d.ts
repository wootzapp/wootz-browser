import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setDefaultRoyaltyInfo" function.
 */
export type SetDefaultRoyaltyInfoParams = WithOverrides<{
    royaltyRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_royaltyRecipient";
    }>;
    royaltyBps: AbiParameterToPrimitiveType<{
        type: "uint16";
        name: "_royaltyBps";
    }>;
}>;
export declare const FN_SELECTOR: "0x93d79445";
/**
 * Checks if the `setDefaultRoyaltyInfo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setDefaultRoyaltyInfo` method is supported.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 *
 * const supported = RoyaltyERC1155.isSetDefaultRoyaltyInfoSupported(["0x..."]);
 * ```
 */
export declare function isSetDefaultRoyaltyInfoSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setDefaultRoyaltyInfo" function.
 * @param options - The options for the setDefaultRoyaltyInfo function.
 * @returns The encoded ABI parameters.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 * const result = RoyaltyERC1155.encodeSetDefaultRoyaltyInfoParams({
 *  royaltyRecipient: ...,
 *  royaltyBps: ...,
 * });
 * ```
 */
export declare function encodeSetDefaultRoyaltyInfoParams(options: SetDefaultRoyaltyInfoParams): `0x${string}`;
/**
 * Encodes the "setDefaultRoyaltyInfo" function into a Hex string with its parameters.
 * @param options - The options for the setDefaultRoyaltyInfo function.
 * @returns The encoded hexadecimal string.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 * const result = RoyaltyERC1155.encodeSetDefaultRoyaltyInfo({
 *  royaltyRecipient: ...,
 *  royaltyBps: ...,
 * });
 * ```
 */
export declare function encodeSetDefaultRoyaltyInfo(options: SetDefaultRoyaltyInfoParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setDefaultRoyaltyInfo" function on the contract.
 * @param options - The options for the "setDefaultRoyaltyInfo" function.
 * @returns A prepared transaction object.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 *
 * const transaction = RoyaltyERC1155.setDefaultRoyaltyInfo({
 *  contract,
 *  royaltyRecipient: ...,
 *  royaltyBps: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setDefaultRoyaltyInfo(options: BaseTransactionOptions<SetDefaultRoyaltyInfoParams | {
    asyncParams: () => Promise<SetDefaultRoyaltyInfoParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setDefaultRoyaltyInfo.d.ts.map