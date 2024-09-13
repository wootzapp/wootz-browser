import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "incrementNonce" function.
 */
export type IncrementNonceParams = WithOverrides<{
    key: AbiParameterToPrimitiveType<{
        type: "uint192";
        name: "key";
    }>;
}>;
export declare const FN_SELECTOR: "0x0bd28e3b";
/**
 * Checks if the `incrementNonce` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `incrementNonce` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isIncrementNonceSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isIncrementNonceSupported(["0x..."]);
 * ```
 */
export declare function isIncrementNonceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "incrementNonce" function.
 * @param options - The options for the incrementNonce function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIncrementNonceParams } from "thirdweb/extensions/erc4337";
 * const result = encodeIncrementNonceParams({
 *  key: ...,
 * });
 * ```
 */
export declare function encodeIncrementNonceParams(options: IncrementNonceParams): `0x${string}`;
/**
 * Encodes the "incrementNonce" function into a Hex string with its parameters.
 * @param options - The options for the incrementNonce function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIncrementNonce } from "thirdweb/extensions/erc4337";
 * const result = encodeIncrementNonce({
 *  key: ...,
 * });
 * ```
 */
export declare function encodeIncrementNonce(options: IncrementNonceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "incrementNonce" function on the contract.
 * @param options - The options for the "incrementNonce" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { incrementNonce } from "thirdweb/extensions/erc4337";
 *
 * const transaction = incrementNonce({
 *  contract,
 *  key: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function incrementNonce(options: BaseTransactionOptions<IncrementNonceParams | {
    asyncParams: () => Promise<IncrementNonceParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=incrementNonce.d.ts.map