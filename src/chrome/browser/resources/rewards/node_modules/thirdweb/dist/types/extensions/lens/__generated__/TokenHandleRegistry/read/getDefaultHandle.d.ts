import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getDefaultHandle" function.
 */
export type GetDefaultHandleParams = {
    profileId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "profileId";
    }>;
};
export declare const FN_SELECTOR: "0xe524488d";
/**
 * Checks if the `getDefaultHandle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getDefaultHandle` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetDefaultHandleSupported } from "thirdweb/extensions/lens";
 * const supported = isGetDefaultHandleSupported(["0x..."]);
 * ```
 */
export declare function isGetDefaultHandleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getDefaultHandle" function.
 * @param options - The options for the getDefaultHandle function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetDefaultHandleParams } from "thirdweb/extensions/lens";
 * const result = encodeGetDefaultHandleParams({
 *  profileId: ...,
 * });
 * ```
 */
export declare function encodeGetDefaultHandleParams(options: GetDefaultHandleParams): `0x${string}`;
/**
 * Encodes the "getDefaultHandle" function into a Hex string with its parameters.
 * @param options - The options for the getDefaultHandle function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetDefaultHandle } from "thirdweb/extensions/lens";
 * const result = encodeGetDefaultHandle({
 *  profileId: ...,
 * });
 * ```
 */
export declare function encodeGetDefaultHandle(options: GetDefaultHandleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getDefaultHandle function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetDefaultHandleResult } from "thirdweb/extensions/lens";
 * const result = decodeGetDefaultHandleResultResult("...");
 * ```
 */
export declare function decodeGetDefaultHandleResult(result: Hex): bigint;
/**
 * Calls the "getDefaultHandle" function on the contract.
 * @param options - The options for the getDefaultHandle function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getDefaultHandle } from "thirdweb/extensions/lens";
 *
 * const result = await getDefaultHandle({
 *  contract,
 *  profileId: ...,
 * });
 *
 * ```
 */
export declare function getDefaultHandle(options: BaseTransactionOptions<GetDefaultHandleParams>): Promise<bigint>;
//# sourceMappingURL=getDefaultHandle.d.ts.map