import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setRoyaltyEngine" function.
 */
export type SetRoyaltyEngineParams = WithOverrides<{
    royaltyEngineAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_royaltyEngineAddress";
    }>;
}>;
export declare const FN_SELECTOR: "0x21ede032";
/**
 * Checks if the `setRoyaltyEngine` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setRoyaltyEngine` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSetRoyaltyEngineSupported } from "thirdweb/extensions/common";
 *
 * const supported = isSetRoyaltyEngineSupported(["0x..."]);
 * ```
 */
export declare function isSetRoyaltyEngineSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setRoyaltyEngine" function.
 * @param options - The options for the setRoyaltyEngine function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetRoyaltyEngineParams } from "thirdweb/extensions/common";
 * const result = encodeSetRoyaltyEngineParams({
 *  royaltyEngineAddress: ...,
 * });
 * ```
 */
export declare function encodeSetRoyaltyEngineParams(options: SetRoyaltyEngineParams): `0x${string}`;
/**
 * Encodes the "setRoyaltyEngine" function into a Hex string with its parameters.
 * @param options - The options for the setRoyaltyEngine function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetRoyaltyEngine } from "thirdweb/extensions/common";
 * const result = encodeSetRoyaltyEngine({
 *  royaltyEngineAddress: ...,
 * });
 * ```
 */
export declare function encodeSetRoyaltyEngine(options: SetRoyaltyEngineParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setRoyaltyEngine" function on the contract.
 * @param options - The options for the "setRoyaltyEngine" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setRoyaltyEngine } from "thirdweb/extensions/common";
 *
 * const transaction = setRoyaltyEngine({
 *  contract,
 *  royaltyEngineAddress: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setRoyaltyEngine(options: BaseTransactionOptions<SetRoyaltyEngineParams | {
    asyncParams: () => Promise<SetRoyaltyEngineParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setRoyaltyEngine.d.ts.map