import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "hasAllRoles" function.
 */
export type HasAllRolesParams = {
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
    }>;
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
    }>;
};
export declare const FN_SELECTOR: "0x1cd64df4";
/**
 * Checks if the `hasAllRoles` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `hasAllRoles` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isHasAllRolesSupported } from "thirdweb/extensions/modules";
 * const supported = isHasAllRolesSupported(["0x..."]);
 * ```
 */
export declare function isHasAllRolesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "hasAllRoles" function.
 * @param options - The options for the hasAllRoles function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeHasAllRolesParams } from "thirdweb/extensions/modules";
 * const result = encodeHasAllRolesParams({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeHasAllRolesParams(options: HasAllRolesParams): `0x${string}`;
/**
 * Encodes the "hasAllRoles" function into a Hex string with its parameters.
 * @param options - The options for the hasAllRoles function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeHasAllRoles } from "thirdweb/extensions/modules";
 * const result = encodeHasAllRoles({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeHasAllRoles(options: HasAllRolesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the hasAllRoles function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeHasAllRolesResult } from "thirdweb/extensions/modules";
 * const result = decodeHasAllRolesResultResult("...");
 * ```
 */
export declare function decodeHasAllRolesResult(result: Hex): boolean;
/**
 * Calls the "hasAllRoles" function on the contract.
 * @param options - The options for the hasAllRoles function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { hasAllRoles } from "thirdweb/extensions/modules";
 *
 * const result = await hasAllRoles({
 *  contract,
 *  user: ...,
 *  roles: ...,
 * });
 *
 * ```
 */
export declare function hasAllRoles(options: BaseTransactionOptions<HasAllRolesParams>): Promise<boolean>;
//# sourceMappingURL=hasAllRoles.d.ts.map