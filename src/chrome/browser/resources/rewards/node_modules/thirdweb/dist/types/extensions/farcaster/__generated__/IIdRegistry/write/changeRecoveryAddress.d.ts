import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "changeRecoveryAddress" function.
 */
export type ChangeRecoveryAddressParams = WithOverrides<{
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
}>;
export declare const FN_SELECTOR: "0xf1f0b224";
/**
 * Checks if the `changeRecoveryAddress` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `changeRecoveryAddress` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isChangeRecoveryAddressSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isChangeRecoveryAddressSupported(["0x..."]);
 * ```
 */
export declare function isChangeRecoveryAddressSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "changeRecoveryAddress" function.
 * @param options - The options for the changeRecoveryAddress function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeChangeRecoveryAddressParams } from "thirdweb/extensions/farcaster";
 * const result = encodeChangeRecoveryAddressParams({
 *  recovery: ...,
 * });
 * ```
 */
export declare function encodeChangeRecoveryAddressParams(options: ChangeRecoveryAddressParams): `0x${string}`;
/**
 * Encodes the "changeRecoveryAddress" function into a Hex string with its parameters.
 * @param options - The options for the changeRecoveryAddress function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeChangeRecoveryAddress } from "thirdweb/extensions/farcaster";
 * const result = encodeChangeRecoveryAddress({
 *  recovery: ...,
 * });
 * ```
 */
export declare function encodeChangeRecoveryAddress(options: ChangeRecoveryAddressParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "changeRecoveryAddress" function on the contract.
 * @param options - The options for the "changeRecoveryAddress" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { changeRecoveryAddress } from "thirdweb/extensions/farcaster";
 *
 * const transaction = changeRecoveryAddress({
 *  contract,
 *  recovery: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function changeRecoveryAddress(options: BaseTransactionOptions<ChangeRecoveryAddressParams | {
    asyncParams: () => Promise<ChangeRecoveryAddressParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=changeRecoveryAddress.d.ts.map