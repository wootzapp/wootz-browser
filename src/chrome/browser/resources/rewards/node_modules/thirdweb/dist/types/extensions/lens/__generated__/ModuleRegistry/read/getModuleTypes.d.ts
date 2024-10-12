import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getModuleTypes" function.
 */
export type GetModuleTypesParams = {
    moduleAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "moduleAddress";
    }>;
};
export declare const FN_SELECTOR: "0xc5dcd896";
/**
 * Checks if the `getModuleTypes` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getModuleTypes` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetModuleTypesSupported } from "thirdweb/extensions/lens";
 * const supported = isGetModuleTypesSupported(["0x..."]);
 * ```
 */
export declare function isGetModuleTypesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getModuleTypes" function.
 * @param options - The options for the getModuleTypes function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetModuleTypesParams } from "thirdweb/extensions/lens";
 * const result = encodeGetModuleTypesParams({
 *  moduleAddress: ...,
 * });
 * ```
 */
export declare function encodeGetModuleTypesParams(options: GetModuleTypesParams): `0x${string}`;
/**
 * Encodes the "getModuleTypes" function into a Hex string with its parameters.
 * @param options - The options for the getModuleTypes function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetModuleTypes } from "thirdweb/extensions/lens";
 * const result = encodeGetModuleTypes({
 *  moduleAddress: ...,
 * });
 * ```
 */
export declare function encodeGetModuleTypes(options: GetModuleTypesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getModuleTypes function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetModuleTypesResult } from "thirdweb/extensions/lens";
 * const result = decodeGetModuleTypesResultResult("...");
 * ```
 */
export declare function decodeGetModuleTypesResult(result: Hex): bigint;
/**
 * Calls the "getModuleTypes" function on the contract.
 * @param options - The options for the getModuleTypes function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getModuleTypes } from "thirdweb/extensions/lens";
 *
 * const result = await getModuleTypes({
 *  contract,
 *  moduleAddress: ...,
 * });
 *
 * ```
 */
export declare function getModuleTypes(options: BaseTransactionOptions<GetModuleTypesParams>): Promise<bigint>;
//# sourceMappingURL=getModuleTypes.d.ts.map