import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getProfileIdByHandleHash" function.
 */
export type GetProfileIdByHandleHashParams = {
    handleHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "handleHash";
    }>;
};
export declare const FN_SELECTOR: "0x19e14070";
/**
 * Checks if the `getProfileIdByHandleHash` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getProfileIdByHandleHash` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetProfileIdByHandleHashSupported } from "thirdweb/extensions/lens";
 * const supported = isGetProfileIdByHandleHashSupported(["0x..."]);
 * ```
 */
export declare function isGetProfileIdByHandleHashSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getProfileIdByHandleHash" function.
 * @param options - The options for the getProfileIdByHandleHash function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetProfileIdByHandleHashParams } from "thirdweb/extensions/lens";
 * const result = encodeGetProfileIdByHandleHashParams({
 *  handleHash: ...,
 * });
 * ```
 */
export declare function encodeGetProfileIdByHandleHashParams(options: GetProfileIdByHandleHashParams): `0x${string}`;
/**
 * Encodes the "getProfileIdByHandleHash" function into a Hex string with its parameters.
 * @param options - The options for the getProfileIdByHandleHash function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetProfileIdByHandleHash } from "thirdweb/extensions/lens";
 * const result = encodeGetProfileIdByHandleHash({
 *  handleHash: ...,
 * });
 * ```
 */
export declare function encodeGetProfileIdByHandleHash(options: GetProfileIdByHandleHashParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getProfileIdByHandleHash function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetProfileIdByHandleHashResult } from "thirdweb/extensions/lens";
 * const result = decodeGetProfileIdByHandleHashResultResult("...");
 * ```
 */
export declare function decodeGetProfileIdByHandleHashResult(result: Hex): bigint;
/**
 * Calls the "getProfileIdByHandleHash" function on the contract.
 * @param options - The options for the getProfileIdByHandleHash function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getProfileIdByHandleHash } from "thirdweb/extensions/lens";
 *
 * const result = await getProfileIdByHandleHash({
 *  contract,
 *  handleHash: ...,
 * });
 *
 * ```
 */
export declare function getProfileIdByHandleHash(options: BaseTransactionOptions<GetProfileIdByHandleHashParams>): Promise<bigint>;
//# sourceMappingURL=getProfileIdByHandleHash.d.ts.map