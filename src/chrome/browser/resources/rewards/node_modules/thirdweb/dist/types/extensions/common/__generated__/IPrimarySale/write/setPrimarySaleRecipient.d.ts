import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setPrimarySaleRecipient" function.
 */
export type SetPrimarySaleRecipientParams = WithOverrides<{
    saleRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_saleRecipient";
    }>;
}>;
export declare const FN_SELECTOR: "0x6f4f2837";
/**
 * Checks if the `setPrimarySaleRecipient` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setPrimarySaleRecipient` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSetPrimarySaleRecipientSupported } from "thirdweb/extensions/common";
 *
 * const supported = isSetPrimarySaleRecipientSupported(["0x..."]);
 * ```
 */
export declare function isSetPrimarySaleRecipientSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setPrimarySaleRecipient" function.
 * @param options - The options for the setPrimarySaleRecipient function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetPrimarySaleRecipientParams } from "thirdweb/extensions/common";
 * const result = encodeSetPrimarySaleRecipientParams({
 *  saleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetPrimarySaleRecipientParams(options: SetPrimarySaleRecipientParams): `0x${string}`;
/**
 * Encodes the "setPrimarySaleRecipient" function into a Hex string with its parameters.
 * @param options - The options for the setPrimarySaleRecipient function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetPrimarySaleRecipient } from "thirdweb/extensions/common";
 * const result = encodeSetPrimarySaleRecipient({
 *  saleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetPrimarySaleRecipient(options: SetPrimarySaleRecipientParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setPrimarySaleRecipient" function on the contract.
 * @param options - The options for the "setPrimarySaleRecipient" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setPrimarySaleRecipient } from "thirdweb/extensions/common";
 *
 * const transaction = setPrimarySaleRecipient({
 *  contract,
 *  saleRecipient: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setPrimarySaleRecipient(options: BaseTransactionOptions<SetPrimarySaleRecipientParams | {
    asyncParams: () => Promise<SetPrimarySaleRecipientParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setPrimarySaleRecipient.d.ts.map