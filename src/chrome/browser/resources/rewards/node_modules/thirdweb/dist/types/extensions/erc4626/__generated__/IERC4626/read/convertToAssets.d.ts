import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "convertToAssets" function.
 */
export type ConvertToAssetsParams = {
    shares: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "shares";
    }>;
};
export declare const FN_SELECTOR: "0x07a2d13a";
/**
 * Checks if the `convertToAssets` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `convertToAssets` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isConvertToAssetsSupported } from "thirdweb/extensions/erc4626";
 * const supported = isConvertToAssetsSupported(["0x..."]);
 * ```
 */
export declare function isConvertToAssetsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "convertToAssets" function.
 * @param options - The options for the convertToAssets function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeConvertToAssetsParams } from "thirdweb/extensions/erc4626";
 * const result = encodeConvertToAssetsParams({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodeConvertToAssetsParams(options: ConvertToAssetsParams): `0x${string}`;
/**
 * Encodes the "convertToAssets" function into a Hex string with its parameters.
 * @param options - The options for the convertToAssets function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeConvertToAssets } from "thirdweb/extensions/erc4626";
 * const result = encodeConvertToAssets({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodeConvertToAssets(options: ConvertToAssetsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the convertToAssets function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeConvertToAssetsResult } from "thirdweb/extensions/erc4626";
 * const result = decodeConvertToAssetsResultResult("...");
 * ```
 */
export declare function decodeConvertToAssetsResult(result: Hex): bigint;
/**
 * Calls the "convertToAssets" function on the contract.
 * @param options - The options for the convertToAssets function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { convertToAssets } from "thirdweb/extensions/erc4626";
 *
 * const result = await convertToAssets({
 *  contract,
 *  shares: ...,
 * });
 *
 * ```
 */
export declare function convertToAssets(options: BaseTransactionOptions<ConvertToAssetsParams>): Promise<bigint>;
//# sourceMappingURL=convertToAssets.d.ts.map