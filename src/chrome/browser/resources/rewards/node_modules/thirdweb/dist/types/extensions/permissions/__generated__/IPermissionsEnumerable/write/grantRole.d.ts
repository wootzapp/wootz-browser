import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "grantRole" function.
 */
export type GrantRoleParams = WithOverrides<{
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
}>;
export declare const FN_SELECTOR: "0x2f2ff15d";
/**
 * Checks if the `grantRole` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `grantRole` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isGrantRoleSupported } from "thirdweb/extensions/permissions";
 *
 * const supported = isGrantRoleSupported(["0x..."]);
 * ```
 */
export declare function isGrantRoleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "grantRole" function.
 * @param options - The options for the grantRole function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGrantRoleParams } from "thirdweb/extensions/permissions";
 * const result = encodeGrantRoleParams({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGrantRoleParams(options: GrantRoleParams): `0x${string}`;
/**
 * Encodes the "grantRole" function into a Hex string with its parameters.
 * @param options - The options for the grantRole function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGrantRole } from "thirdweb/extensions/permissions";
 * const result = encodeGrantRole({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGrantRole(options: GrantRoleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "grantRole" function on the contract.
 * @param options - The options for the "grantRole" function.
 * @returns A prepared transaction object.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { grantRole } from "thirdweb/extensions/permissions";
 *
 * const transaction = grantRole({
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
export declare function grantRole(options: BaseTransactionOptions<GrantRoleParams | {
    asyncParams: () => Promise<GrantRoleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=grantRole.d.ts.map