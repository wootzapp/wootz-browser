import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "register" function.
 */
export type RegisterParams = WithOverrides<{
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
    extraStorage: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "extraStorage";
    }>;
}>;
export declare const FN_SELECTOR: "0x6d705ebb";
/**
 * Checks if the `register` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `register` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRegisterSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRegisterSupported(["0x..."]);
 * ```
 */
export declare function isRegisterSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "register" function.
 * @param options - The options for the register function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRegisterParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRegisterParams({
 *  recovery: ...,
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodeRegisterParams(options: RegisterParams): `0x${string}`;
/**
 * Encodes the "register" function into a Hex string with its parameters.
 * @param options - The options for the register function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRegister } from "thirdweb/extensions/farcaster";
 * const result = encodeRegister({
 *  recovery: ...,
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodeRegister(options: RegisterParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "register" function on the contract.
 * @param options - The options for the "register" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { register } from "thirdweb/extensions/farcaster";
 *
 * const transaction = register({
 *  contract,
 *  recovery: ...,
 *  extraStorage: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function register(options: BaseTransactionOptions<RegisterParams | {
    asyncParams: () => Promise<RegisterParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=register.d.ts.map