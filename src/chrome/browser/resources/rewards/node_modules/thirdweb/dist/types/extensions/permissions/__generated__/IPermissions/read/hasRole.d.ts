import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "hasRole" function.
 */
export type HasRoleParams = {
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x91d14854";
/**
 * Checks if the `hasRole` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `hasRole` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isHasRoleSupported } from "thirdweb/extensions/permissions";
 * const supported = isHasRoleSupported(["0x..."]);
 * ```
 */
export declare function isHasRoleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "hasRole" function.
 * @param options - The options for the hasRole function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeHasRoleParams } from "thirdweb/extensions/permissions";
 * const result = encodeHasRoleParams({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeHasRoleParams(options: HasRoleParams): `0x${string}`;
/**
 * Encodes the "hasRole" function into a Hex string with its parameters.
 * @param options - The options for the hasRole function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeHasRole } from "thirdweb/extensions/permissions";
 * const result = encodeHasRole({
 *  role: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeHasRole(options: HasRoleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the hasRole function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { decodeHasRoleResult } from "thirdweb/extensions/permissions";
 * const result = decodeHasRoleResultResult("...");
 * ```
 */
export declare function decodeHasRoleResult(result: Hex): boolean;
/**
 * Calls the "hasRole" function on the contract.
 * @param options - The options for the hasRole function.
 * @returns The parsed result of the function call.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { hasRole } from "thirdweb/extensions/permissions";
 *
 * const result = await hasRole({
 *  contract,
 *  role: ...,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function hasRole(options: BaseTransactionOptions<HasRoleParams>): Promise<boolean>;
//# sourceMappingURL=hasRole.d.ts.map