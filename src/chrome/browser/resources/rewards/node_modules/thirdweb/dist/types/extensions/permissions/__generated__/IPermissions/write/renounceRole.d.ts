import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "renounceRole" function.
 */
export type RenounceRoleParams = WithOverrides<{
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
}>;
export declare const FN_SELECTOR: "0x36568abe";
/**
 * Checks if the `renounceRole` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `renounceRole` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isRenounceRoleSupported } from "thirdweb/extensions/permissions";
 *
 * const supported = isRenounceRoleSupported(["0x..."]);
 * ```
 */
export declare function isRenounceRoleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "renounceRole" function.
 * @param options - The options for the renounceRole function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeRenounceRoleParams } from "thirdweb/extensions/permissions";
 * const result = encodeRenounceRoleParams({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeRenounceRoleParams(options: RenounceRoleParams): `0x${string}`;
/**
 * Encodes the "renounceRole" function into a Hex string with its parameters.
 * @param options - The options for the renounceRole function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeRenounceRole } from "thirdweb/extensions/permissions";
 * const result = encodeRenounceRole({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeRenounceRole(options: RenounceRoleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "renounceRole" function on the contract.
 * @param options - The options for the "renounceRole" function.
 * @returns A prepared transaction object.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { renounceRole } from "thirdweb/extensions/permissions";
 *
 * const transaction = renounceRole({
 *  contract,
 *  role: ...,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function renounceRole(options: BaseTransactionOptions<RenounceRoleParams | {
    asyncParams: () => Promise<RenounceRoleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=renounceRole.d.ts.map