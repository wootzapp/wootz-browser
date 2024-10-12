import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "revokeRoles" function.
 */
export type RevokeRolesParams = WithOverrides<{
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
    }>;
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
    }>;
}>;
export declare const FN_SELECTOR: "0x4a4ee7b1";
/**
 * Checks if the `revokeRoles` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `revokeRoles` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRevokeRolesSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isRevokeRolesSupported(["0x..."]);
 * ```
 */
export declare function isRevokeRolesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "revokeRoles" function.
 * @param options - The options for the revokeRoles function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRevokeRolesParams } from "thirdweb/extensions/modules";
 * const result = encodeRevokeRolesParams({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeRevokeRolesParams(options: RevokeRolesParams): `0x${string}`;
/**
 * Encodes the "revokeRoles" function into a Hex string with its parameters.
 * @param options - The options for the revokeRoles function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRevokeRoles } from "thirdweb/extensions/modules";
 * const result = encodeRevokeRoles({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeRevokeRoles(options: RevokeRolesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "revokeRoles" function on the contract.
 * @param options - The options for the "revokeRoles" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { revokeRoles } from "thirdweb/extensions/modules";
 *
 * const transaction = revokeRoles({
 *  contract,
 *  user: ...,
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
export declare function revokeRoles(options: BaseTransactionOptions<RevokeRolesParams | {
    asyncParams: () => Promise<RevokeRolesParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=revokeRoles.d.ts.map