import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "supportsInterface" function.
 */
export type SupportsInterfaceParams = {
    interfaceId: AbiParameterToPrimitiveType<{
        type: "bytes4";
        name: "interfaceId";
    }>;
};
export declare const FN_SELECTOR: "0x01ffc9a7";
/**
 * Checks if the `supportsInterface` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `supportsInterface` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSupportsInterfaceSupported } from "thirdweb/extensions/common";
 * const supported = isSupportsInterfaceSupported(["0x..."]);
 * ```
 */
export declare function isSupportsInterfaceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "supportsInterface" function.
 * @param options - The options for the supportsInterface function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSupportsInterfaceParams } from "thirdweb/extensions/common";
 * const result = encodeSupportsInterfaceParams({
 *  interfaceId: ...,
 * });
 * ```
 */
export declare function encodeSupportsInterfaceParams(options: SupportsInterfaceParams): `0x${string}`;
/**
 * Encodes the "supportsInterface" function into a Hex string with its parameters.
 * @param options - The options for the supportsInterface function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSupportsInterface } from "thirdweb/extensions/common";
 * const result = encodeSupportsInterface({
 *  interfaceId: ...,
 * });
 * ```
 */
export declare function encodeSupportsInterface(options: SupportsInterfaceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the supportsInterface function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension COMMON
 * @example
 * ```ts
 * import { decodeSupportsInterfaceResult } from "thirdweb/extensions/common";
 * const result = decodeSupportsInterfaceResultResult("...");
 * ```
 */
export declare function decodeSupportsInterfaceResult(result: Hex): boolean;
/**
 * Calls the "supportsInterface" function on the contract.
 * @param options - The options for the supportsInterface function.
 * @returns The parsed result of the function call.
 * @extension COMMON
 * @example
 * ```ts
 * import { supportsInterface } from "thirdweb/extensions/common";
 *
 * const result = await supportsInterface({
 *  contract,
 *  interfaceId: ...,
 * });
 *
 * ```
 */
export declare function supportsInterface(options: BaseTransactionOptions<SupportsInterfaceParams>): Promise<boolean>;
//# sourceMappingURL=supportsInterface.d.ts.map