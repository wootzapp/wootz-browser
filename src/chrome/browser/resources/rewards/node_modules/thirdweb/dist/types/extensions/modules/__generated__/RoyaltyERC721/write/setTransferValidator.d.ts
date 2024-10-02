import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setTransferValidator" function.
 */
export type SetTransferValidatorParams = WithOverrides<{
    validator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "validator";
    }>;
}>;
export declare const FN_SELECTOR: "0xa9fc664e";
/**
 * Checks if the `setTransferValidator` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setTransferValidator` method is supported.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { RoyaltyERC721 } from "thirdweb/modules";
 *
 * const supported = RoyaltyERC721.isSetTransferValidatorSupported(["0x..."]);
 * ```
 */
export declare function isSetTransferValidatorSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setTransferValidator" function.
 * @param options - The options for the setTransferValidator function.
 * @returns The encoded ABI parameters.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { RoyaltyERC721 } from "thirdweb/modules";
 * const result = RoyaltyERC721.encodeSetTransferValidatorParams({
 *  validator: ...,
 * });
 * ```
 */
export declare function encodeSetTransferValidatorParams(options: SetTransferValidatorParams): `0x${string}`;
/**
 * Encodes the "setTransferValidator" function into a Hex string with its parameters.
 * @param options - The options for the setTransferValidator function.
 * @returns The encoded hexadecimal string.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { RoyaltyERC721 } from "thirdweb/modules";
 * const result = RoyaltyERC721.encodeSetTransferValidator({
 *  validator: ...,
 * });
 * ```
 */
export declare function encodeSetTransferValidator(options: SetTransferValidatorParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setTransferValidator" function on the contract.
 * @param options - The options for the "setTransferValidator" function.
 * @returns A prepared transaction object.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { RoyaltyERC721 } from "thirdweb/modules";
 *
 * const transaction = RoyaltyERC721.setTransferValidator({
 *  contract,
 *  validator: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setTransferValidator(options: BaseTransactionOptions<SetTransferValidatorParams | {
    asyncParams: () => Promise<SetTransferValidatorParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setTransferValidator.d.ts.map