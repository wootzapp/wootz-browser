import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "changeRecoveryAddressFor" function.
 */
export type ChangeRecoveryAddressForParams = WithOverrides<{
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
}>;
export declare const FN_SELECTOR: "0x9cbef8dc";
/**
 * Checks if the `changeRecoveryAddressFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `changeRecoveryAddressFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isChangeRecoveryAddressForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isChangeRecoveryAddressForSupported(["0x..."]);
 * ```
 */
export declare function isChangeRecoveryAddressForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "changeRecoveryAddressFor" function.
 * @param options - The options for the changeRecoveryAddressFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeChangeRecoveryAddressForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeChangeRecoveryAddressForParams({
 *  owner: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeChangeRecoveryAddressForParams(options: ChangeRecoveryAddressForParams): `0x${string}`;
/**
 * Encodes the "changeRecoveryAddressFor" function into a Hex string with its parameters.
 * @param options - The options for the changeRecoveryAddressFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeChangeRecoveryAddressFor } from "thirdweb/extensions/farcaster";
 * const result = encodeChangeRecoveryAddressFor({
 *  owner: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeChangeRecoveryAddressFor(options: ChangeRecoveryAddressForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "changeRecoveryAddressFor" function on the contract.
 * @param options - The options for the "changeRecoveryAddressFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { changeRecoveryAddressFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = changeRecoveryAddressFor({
 *  contract,
 *  owner: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function changeRecoveryAddressFor(options: BaseTransactionOptions<ChangeRecoveryAddressForParams | {
    asyncParams: () => Promise<ChangeRecoveryAddressForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=changeRecoveryAddressFor.d.ts.map