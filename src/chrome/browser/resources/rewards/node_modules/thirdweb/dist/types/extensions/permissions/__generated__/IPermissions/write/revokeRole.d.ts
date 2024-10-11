import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "revokeRole" function.
 */
export type RevokeRoleParams = WithOverrides<{
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
}>;
export declare const FN_SELECTOR: "0xd547741f";
/**
 * Checks if the `revokeRole` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `revokeRole` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isRevokeRoleSupported } from "thirdweb/extensions/permissions";
 *
 * const supported = isRevokeRoleSupported(["0x..."]);
 * ```
 */
export declare function isRevokeRoleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "revokeRole" function.
 * @param options - The options for the revokeRole function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeRevokeRoleParams } from "thirdweb/extensions/permissions";
 * const result = encodeRevokeRoleParams({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeRevokeRoleParams(options: RevokeRoleParams): `0x${string}`;
/**
 * Encodes the "revokeRole" function into a Hex string with its parameters.
 * @param options - The options for the revokeRole function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeRevokeRole } from "thirdweb/extensions/permissions";
 * const result = encodeRevokeRole({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeRevokeRole(options: RevokeRoleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "revokeRole" function on the contract.
 * @param options - The options for the "revokeRole" function.
 * @returns A prepared transaction object.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { revokeRole } from "thirdweb/extensions/permissions";
 *
 * const transaction = revokeRole({
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
export declare function revokeRole(options: BaseTransactionOptions<RevokeRoleParams | {
    asyncParams: () => Promise<RevokeRoleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=revokeRole.d.ts.map