import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "price" function.
 */
export type PriceParams = {
    extraStorage: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "extraStorage";
    }>;
};
export declare const FN_SELECTOR: "0x26a49e37";
/**
 * Checks if the `price` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `price` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isPriceSupported } from "thirdweb/extensions/farcaster";
 * const supported = isPriceSupported(["0x..."]);
 * ```
 */
export declare function isPriceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "price" function.
 * @param options - The options for the price function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodePriceParams } from "thirdweb/extensions/farcaster";
 * const result = encodePriceParams({
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodePriceParams(options: PriceParams): `0x${string}`;
/**
 * Encodes the "price" function into a Hex string with its parameters.
 * @param options - The options for the price function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodePrice } from "thirdweb/extensions/farcaster";
 * const result = encodePrice({
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodePrice(options: PriceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the price function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodePriceResult } from "thirdweb/extensions/farcaster";
 * const result = decodePriceResultResult("...");
 * ```
 */
export declare function decodePriceResult(result: Hex): bigint;
/**
 * Calls the "price" function on the contract.
 * @param options - The options for the price function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { price } from "thirdweb/extensions/farcaster";
 *
 * const result = await price({
 *  contract,
 *  extraStorage: ...,
 * });
 *
 * ```
 */
export declare function price(options: BaseTransactionOptions<PriceParams>): Promise<bigint>;
//# sourceMappingURL=price.d.ts.map