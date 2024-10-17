import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createAccount" function.
 */
export type CreateAccountParams = WithOverrides<{
    admin: AbiParameterToPrimitiveType<{
        type: "address";
        name: "admin";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "_data";
    }>;
}>;
export declare const FN_SELECTOR: "0xd8fd8f44";
/**
 * Checks if the `createAccount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createAccount` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isCreateAccountSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isCreateAccountSupported(["0x..."]);
 * ```
 */
export declare function isCreateAccountSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createAccount" function.
 * @param options - The options for the createAccount function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeCreateAccountParams } from "thirdweb/extensions/erc4337";
 * const result = encodeCreateAccountParams({
 *  admin: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeCreateAccountParams(options: CreateAccountParams): `0x${string}`;
/**
 * Encodes the "createAccount" function into a Hex string with its parameters.
 * @param options - The options for the createAccount function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeCreateAccount } from "thirdweb/extensions/erc4337";
 * const result = encodeCreateAccount({
 *  admin: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeCreateAccount(options: CreateAccountParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createAccount" function on the contract.
 * @param options - The options for the "createAccount" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createAccount } from "thirdweb/extensions/erc4337";
 *
 * const transaction = createAccount({
 *  contract,
 *  admin: ...,
 *  data: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createAccount(options: BaseTransactionOptions<CreateAccountParams | {
    asyncParams: () => Promise<CreateAccountParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createAccount.d.ts.map