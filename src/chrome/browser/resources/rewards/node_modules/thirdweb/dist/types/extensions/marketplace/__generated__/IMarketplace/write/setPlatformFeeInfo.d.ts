import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setPlatformFeeInfo" function.
 */
export type SetPlatformFeeInfoParams = WithOverrides<{
    platformFeeRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_platformFeeRecipient";
    }>;
    platformFeeBps: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_platformFeeBps";
    }>;
}>;
export declare const FN_SELECTOR: "0x1e7ac488";
/**
 * Checks if the `setPlatformFeeInfo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setPlatformFeeInfo` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isSetPlatformFeeInfoSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isSetPlatformFeeInfoSupported(["0x..."]);
 * ```
 */
export declare function isSetPlatformFeeInfoSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setPlatformFeeInfo" function.
 * @param options - The options for the setPlatformFeeInfo function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeSetPlatformFeeInfoParams } from "thirdweb/extensions/marketplace";
 * const result = encodeSetPlatformFeeInfoParams({
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 * });
 * ```
 */
export declare function encodeSetPlatformFeeInfoParams(options: SetPlatformFeeInfoParams): `0x${string}`;
/**
 * Encodes the "setPlatformFeeInfo" function into a Hex string with its parameters.
 * @param options - The options for the setPlatformFeeInfo function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeSetPlatformFeeInfo } from "thirdweb/extensions/marketplace";
 * const result = encodeSetPlatformFeeInfo({
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 * });
 * ```
 */
export declare function encodeSetPlatformFeeInfo(options: SetPlatformFeeInfoParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setPlatformFeeInfo" function on the contract.
 * @param options - The options for the "setPlatformFeeInfo" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setPlatformFeeInfo } from "thirdweb/extensions/marketplace";
 *
 * const transaction = setPlatformFeeInfo({
 *  contract,
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setPlatformFeeInfo(options: BaseTransactionOptions<SetPlatformFeeInfoParams | {
    asyncParams: () => Promise<SetPlatformFeeInfoParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setPlatformFeeInfo.d.ts.map