import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getRoleMemberCount" function.
 */
export type GetRoleMemberCountParams = {
    role: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "role";
    }>;
};
export declare const FN_SELECTOR: "0xca15c873";
/**
 * Checks if the `getRoleMemberCount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getRoleMemberCount` method is supported.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { isGetRoleMemberCountSupported } from "thirdweb/extensions/permissions";
 * const supported = isGetRoleMemberCountSupported(["0x..."]);
 * ```
 */
export declare function isGetRoleMemberCountSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getRoleMemberCount" function.
 * @param options - The options for the getRoleMemberCount function.
 * @returns The encoded ABI parameters.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGetRoleMemberCountParams } from "thirdweb/extensions/permissions";
 * const result = encodeGetRoleMemberCountParams({
 *  role: ...,
 * });
 * ```
 */
export declare function encodeGetRoleMemberCountParams(options: GetRoleMemberCountParams): `0x${string}`;
/**
 * Encodes the "getRoleMemberCount" function into a Hex string with its parameters.
 * @param options - The options for the getRoleMemberCount function.
 * @returns The encoded hexadecimal string.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { encodeGetRoleMemberCount } from "thirdweb/extensions/permissions";
 * const result = encodeGetRoleMemberCount({
 *  role: ...,
 * });
 * ```
 */
export declare function encodeGetRoleMemberCount(options: GetRoleMemberCountParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getRoleMemberCount function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { decodeGetRoleMemberCountResult } from "thirdweb/extensions/permissions";
 * const result = decodeGetRoleMemberCountResultResult("...");
 * ```
 */
export declare function decodeGetRoleMemberCountResult(result: Hex): bigint;
/**
 * Calls the "getRoleMemberCount" function on the contract.
 * @param options - The options for the getRoleMemberCount function.
 * @returns The parsed result of the function call.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { getRoleMemberCount } from "thirdweb/extensions/permissions";
 *
 * const result = await getRoleMemberCount({
 *  contract,
 *  role: ...,
 * });
 *
 * ```
 */
export declare function getRoleMemberCount(options: BaseTransactionOptions<GetRoleMemberCountParams>): Promise<bigint>;
//# sourceMappingURL=getRoleMemberCount.d.ts.map