import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "convertToShares" function.
 */
export type ConvertToSharesParams = {
    assets: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "assets";
    }>;
};
export declare const FN_SELECTOR: "0xc6e6f592";
/**
 * Checks if the `convertToShares` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `convertToShares` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isConvertToSharesSupported } from "thirdweb/extensions/erc4626";
 * const supported = isConvertToSharesSupported(["0x..."]);
 * ```
 */
export declare function isConvertToSharesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "convertToShares" function.
 * @param options - The options for the convertToShares function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeConvertToSharesParams } from "thirdweb/extensions/erc4626";
 * const result = encodeConvertToSharesParams({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodeConvertToSharesParams(options: ConvertToSharesParams): `0x${string}`;
/**
 * Encodes the "convertToShares" function into a Hex string with its parameters.
 * @param options - The options for the convertToShares function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeConvertToShares } from "thirdweb/extensions/erc4626";
 * const result = encodeConvertToShares({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodeConvertToShares(options: ConvertToSharesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the convertToShares function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeConvertToSharesResult } from "thirdweb/extensions/erc4626";
 * const result = decodeConvertToSharesResultResult("...");
 * ```
 */
export declare function decodeConvertToSharesResult(result: Hex): bigint;
/**
 * Calls the "convertToShares" function on the contract.
 * @param options - The options for the convertToShares function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { convertToShares } from "thirdweb/extensions/erc4626";
 *
 * const result = await convertToShares({
 *  contract,
 *  assets: ...,
 * });
 *
 * ```
 */
export declare function convertToShares(options: BaseTransactionOptions<ConvertToSharesParams>): Promise<bigint>;
//# sourceMappingURL=convertToShares.d.ts.map