import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "rolesOf" function.
 */
export type RolesOfParams = {
    user: AbiParameterToPrimitiveType<{
        type: "address";
        name: "user";
    }>;
};
export declare const FN_SELECTOR: "0x2de94807";
/**
 * Checks if the `rolesOf` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `rolesOf` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isRolesOfSupported } from "thirdweb/extensions/modules";
 * const supported = isRolesOfSupported(["0x..."]);
 * ```
 */
export declare function isRolesOfSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "rolesOf" function.
 * @param options - The options for the rolesOf function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRolesOfParams } from "thirdweb/extensions/modules";
 * const result = encodeRolesOfParams({
 *  user: ...,
 * });
 * ```
 */
export declare function encodeRolesOfParams(options: RolesOfParams): `0x${string}`;
/**
 * Encodes the "rolesOf" function into a Hex string with its parameters.
 * @param options - The options for the rolesOf function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeRolesOf } from "thirdweb/extensions/modules";
 * const result = encodeRolesOf({
 *  user: ...,
 * });
 * ```
 */
export declare function encodeRolesOf(options: RolesOfParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the rolesOf function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeRolesOfResult } from "thirdweb/extensions/modules";
 * const result = decodeRolesOfResultResult("...");
 * ```
 */
export declare function decodeRolesOfResult(result: Hex): bigint;
/**
 * Calls the "rolesOf" function on the contract.
 * @param options - The options for the rolesOf function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { rolesOf } from "thirdweb/extensions/modules";
 *
 * const result = await rolesOf({
 *  contract,
 *  user: ...,
 * });
 *
 * ```
 */
export declare function rolesOf(options: BaseTransactionOptions<RolesOfParams>): Promise<bigint>;
//# sourceMappingURL=rolesOf.d.ts.map