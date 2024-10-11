import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "hasAnyRole" function.
 */
export type HasAnyRoleParams = {
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
    }>;
    roles: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "roles";
    }>;
};
export declare const FN_SELECTOR: "0x514e62fc";
/**
 * Checks if the `hasAnyRole` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `hasAnyRole` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isHasAnyRoleSupported } from "thirdweb/extensions/modules";
 * const supported = isHasAnyRoleSupported(["0x..."]);
 * ```
 */
export declare function isHasAnyRoleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "hasAnyRole" function.
 * @param options - The options for the hasAnyRole function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeHasAnyRoleParams } from "thirdweb/extensions/modules";
 * const result = encodeHasAnyRoleParams({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeHasAnyRoleParams(options: HasAnyRoleParams): `0x${string}`;
/**
 * Encodes the "hasAnyRole" function into a Hex string with its parameters.
 * @param options - The options for the hasAnyRole function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeHasAnyRole } from "thirdweb/extensions/modules";
 * const result = encodeHasAnyRole({
 *  user: ...,
 *  roles: ...,
 * });
 * ```
 */
export declare function encodeHasAnyRole(options: HasAnyRoleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the hasAnyRole function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeHasAnyRoleResult } from "thirdweb/extensions/modules";
 * const result = decodeHasAnyRoleResultResult("...");
 * ```
 */
export declare function decodeHasAnyRoleResult(result: Hex): boolean;
/**
 * Calls the "hasAnyRole" function on the contract.
 * @param options - The options for the hasAnyRole function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { hasAnyRole } from "thirdweb/extensions/modules";
 *
 * const result = await hasAnyRole({
 *  contract,
 *  user: ...,
 *  roles: ...,
 * });
 *
 * ```
 */
export declare function hasAnyRole(options: BaseTransactionOptions<HasAnyRoleParams>): Promise<boolean>;
//# sourceMappingURL=hasAnyRole.d.ts.map