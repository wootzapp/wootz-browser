import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "grantRoles" function.
 */
export type GrantRolesParams = WithOverrides<{
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
    }>;
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
    }>;
}>;
export declare const FN_SELECTOR: "0x1c10893f";
/**
 * Checks if the `grantRoles` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `grantRoles` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isGrantRolesSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isGrantRolesSupported(["0x..."]);
 * ```
 */
export declare function isGrantRolesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "grantRoles" function.
 * @param options - The options for the grantRoles function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeGrantRolesParams } from "thirdweb/extensions/modules";
 * const result = encodeGrantRolesParams({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeGrantRolesParams(options: GrantRolesParams): `0x${string}`;
/**
 * Encodes the "grantRoles" function into a Hex string with its parameters.
 * @param options - The options for the grantRoles function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeGrantRoles } from "thirdweb/extensions/modules";
 * const result = encodeGrantRoles({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeGrantRoles(options: GrantRolesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "grantRoles" function on the contract.
 * @param options - The options for the "grantRoles" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { grantRoles } from "thirdweb/extensions/modules";
 *
 * const transaction = grantRoles({
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
export declare function grantRoles(options: BaseTransactionOptions<GrantRolesParams | {
    asyncParams: () => Promise<GrantRolesParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=grantRoles.d.ts.map