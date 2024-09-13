import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "renounceRoles" function.
 */
export type RenounceRolesParams = WithOverrides<{
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
    }>;
}>;
export declare const FN_SELECTOR: "0x183a4f6e";
/**
 * Checks if the `renounceRoles` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `renounceRoles` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRenounceRolesSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRenounceRolesSupported(["0x..."]);
 * ```
 */
export declare function isRenounceRolesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "renounceRoles" function.
 * @param options - The options for the renounceRoles function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRenounceRolesParams } from "thirdweb/extensions/modules";
 * const result = encodeRenounceRolesParams({
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeRenounceRolesParams(options: RenounceRolesParams): `0x${string}`;
/**
 * Encodes the "renounceRoles" function into a Hex string with its parameters.
 * @param options - The options for the renounceRoles function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRenounceRoles } from "thirdweb/extensions/modules";
 * const result = encodeRenounceRoles({
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeRenounceRoles(options: RenounceRolesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "renounceRoles" function on the contract.
 * @param options - The options for the "renounceRoles" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { renounceRoles } from "thirdweb/extensions/modules";
 *
 * const transaction = renounceRoles({
 *  contract,
 *  roles: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function renounceRoles(options: BaseTransactionOptions<RenounceRolesParams | {
    asyncParams: () => Promise<RenounceRolesParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=renounceRoles.d.ts.map