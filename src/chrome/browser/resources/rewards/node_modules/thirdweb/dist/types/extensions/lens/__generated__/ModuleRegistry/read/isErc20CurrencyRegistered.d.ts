import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isErc20CurrencyRegistered" function.
 */
export type IsErc20CurrencyRegisteredParams = {
    currencyAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "currencyAddress";
    }>;
};
export declare const FN_SELECTOR: "0xf21b24d7";
/**
 * Checks if the `isErc20CurrencyRegistered` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isErc20CurrencyRegistered` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isIsErc20CurrencyRegisteredSupported } from "thirdweb/extensions/lens";
 * const supported = isIsErc20CurrencyRegisteredSupported(["0x..."]);
 * ```
 */
export declare function isIsErc20CurrencyRegisteredSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isErc20CurrencyRegistered" function.
 * @param options - The options for the isErc20CurrencyRegistered function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsErc20CurrencyRegisteredParams } from "thirdweb/extensions/lens";
 * const result = encodeIsErc20CurrencyRegisteredParams({
 *  currencyAddress: ...,
 * });
 * ```
 */
export declare function encodeIsErc20CurrencyRegisteredParams(options: IsErc20CurrencyRegisteredParams): `0x${string}`;
/**
 * Encodes the "isErc20CurrencyRegistered" function into a Hex string with its parameters.
 * @param options - The options for the isErc20CurrencyRegistered function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsErc20CurrencyRegistered } from "thirdweb/extensions/lens";
 * const result = encodeIsErc20CurrencyRegistered({
 *  currencyAddress: ...,
 * });
 * ```
 */
export declare function encodeIsErc20CurrencyRegistered(options: IsErc20CurrencyRegisteredParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isErc20CurrencyRegistered function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeIsErc20CurrencyRegisteredResult } from "thirdweb/extensions/lens";
 * const result = decodeIsErc20CurrencyRegisteredResultResult("...");
 * ```
 */
export declare function decodeIsErc20CurrencyRegisteredResult(result: Hex): boolean;
/**
 * Calls the "isErc20CurrencyRegistered" function on the contract.
 * @param options - The options for the isErc20CurrencyRegistered function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { isErc20CurrencyRegistered } from "thirdweb/extensions/lens";
 *
 * const result = await isErc20CurrencyRegistered({
 *  contract,
 *  currencyAddress: ...,
 * });
 *
 * ```
 */
export declare function isErc20CurrencyRegistered(options: BaseTransactionOptions<IsErc20CurrencyRegisteredParams>): Promise<boolean>;
//# sourceMappingURL=isErc20CurrencyRegistered.d.ts.map