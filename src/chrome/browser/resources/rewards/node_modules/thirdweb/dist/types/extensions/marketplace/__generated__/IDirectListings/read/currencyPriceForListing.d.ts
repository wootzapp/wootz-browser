import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "currencyPriceForListing" function.
 */
export type CurrencyPriceForListingParams = {
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currency";
    }>;
};
export declare const FN_SELECTOR: "0xfb14079d";
/**
 * Checks if the `currencyPriceForListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `currencyPriceForListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCurrencyPriceForListingSupported } from "thirdweb/extensions/marketplace";
 * const supported = isCurrencyPriceForListingSupported(["0x..."]);
 * ```
 */
export declare function isCurrencyPriceForListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "currencyPriceForListing" function.
 * @param options - The options for the currencyPriceForListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCurrencyPriceForListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCurrencyPriceForListingParams({
 *  listingId: ...,
 *  currency: ...,
 * });
 * ```
 */
export declare function encodeCurrencyPriceForListingParams(options: CurrencyPriceForListingParams): `0x${string}`;
/**
 * Encodes the "currencyPriceForListing" function into a Hex string with its parameters.
 * @param options - The options for the currencyPriceForListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCurrencyPriceForListing } from "thirdweb/extensions/marketplace";
 * const result = encodeCurrencyPriceForListing({
 *  listingId: ...,
 *  currency: ...,
 * });
 * ```
 */
export declare function encodeCurrencyPriceForListing(options: CurrencyPriceForListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the currencyPriceForListing function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeCurrencyPriceForListingResult } from "thirdweb/extensions/marketplace";
 * const result = decodeCurrencyPriceForListingResultResult("...");
 * ```
 */
export declare function decodeCurrencyPriceForListingResult(result: Hex): bigint;
/**
 * Calls the "currencyPriceForListing" function on the contract.
 * @param options - The options for the currencyPriceForListing function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { currencyPriceForListing } from "thirdweb/extensions/marketplace";
 *
 * const result = await currencyPriceForListing({
 *  contract,
 *  listingId: ...,
 *  currency: ...,
 * });
 *
 * ```
 */
export declare function currencyPriceForListing(options: BaseTransactionOptions<CurrencyPriceForListingParams>): Promise<bigint>;
//# sourceMappingURL=currencyPriceForListing.d.ts.map