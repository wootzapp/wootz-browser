import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getRoleAdmin" function.
 */
export type GetRoleAdminParams = {
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
};
export declare const FN_SELECTOR: "0x248a9ca3";
/**
 * Checks if the `getRoleAdmin` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getRoleAdmin` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isGetRoleAdminSupported } from "thirdweb/extensions/permissions";
 * const supported = isGetRoleAdminSupported(["0x..."]);
 * ```
 */
export declare function isGetRoleAdminSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getRoleAdmin" function.
 * @param options - The options for the getRoleAdmin function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGetRoleAdminParams } from "thirdweb/extensions/permissions";
 * const result = encodeGetRoleAdminParams({
 *  role: ...,
 * });
 * ```
 */
export declare function encodeGetRoleAdminParams(options: GetRoleAdminParams): `0x${string}`;
/**
 * Encodes the "getRoleAdmin" function into a Hex string with its parameters.
 * @param options - The options for the getRoleAdmin function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGetRoleAdmin } from "thirdweb/extensions/permissions";
 * const result = encodeGetRoleAdmin({
 *  role: ...,
 * });
 * ```
 */
export declare function encodeGetRoleAdmin(options: GetRoleAdminParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getRoleAdmin function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { decodeGetRoleAdminResult } from "thirdweb/extensions/permissions";
 * const result = decodeGetRoleAdminResultResult("...");
 * ```
 */
export declare function decodeGetRoleAdminResult(result: Hex): `0x${string}`;
/**
 * Calls the "getRoleAdmin" function on the contract.
 * @param options - The options for the getRoleAdmin function.
 * @returns The parsed result of the function call.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { getRoleAdmin } from "thirdweb/extensions/permissions";
 *
 * const result = await getRoleAdmin({
 *  contract,
 *  role: ...,
 * });
 *
 * ```
 */
export declare function getRoleAdmin(options: BaseTransactionOptions<GetRoleAdminParams>): Promise<`0x${string}`>;
//# sourceMappingURL=getRoleAdmin.d.ts.map