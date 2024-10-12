import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isCurrencyApprovedForListing" function.
 */
export type IsCurrencyApprovedForListingParams = {
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currency";
    }>;
};
export declare const FN_SELECTOR: "0xa8519047";
/**
 * Checks if the `isCurrencyApprovedForListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isCurrencyApprovedForListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isIsCurrencyApprovedForListingSupported } from "thirdweb/extensions/marketplace";
 * const supported = isIsCurrencyApprovedForListingSupported(["0x..."]);
 * ```
 */
export declare function isIsCurrencyApprovedForListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isCurrencyApprovedForListing" function.
 * @param options - The options for the isCurrencyApprovedForListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsCurrencyApprovedForListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeIsCurrencyApprovedForListingParams({
 *  listingId: ...,
 *  currency: ...,
 * });
 * ```
 */
export declare function encodeIsCurrencyApprovedForListingParams(options: IsCurrencyApprovedForListingParams): `0x${string}`;
/**
 * Encodes the "isCurrencyApprovedForListing" function into a Hex string with its parameters.
 * @param options - The options for the isCurrencyApprovedForListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsCurrencyApprovedForListing } from "thirdweb/extensions/marketplace";
 * const result = encodeIsCurrencyApprovedForListing({
 *  listingId: ...,
 *  currency: ...,
 * });
 * ```
 */
export declare function encodeIsCurrencyApprovedForListing(options: IsCurrencyApprovedForListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isCurrencyApprovedForListing function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeIsCurrencyApprovedForListingResult } from "thirdweb/extensions/marketplace";
 * const result = decodeIsCurrencyApprovedForListingResultResult("...");
 * ```
 */
export declare function decodeIsCurrencyApprovedForListingResult(result: Hex): boolean;
/**
 * Calls the "isCurrencyApprovedForListing" function on the contract.
 * @param options - The options for the isCurrencyApprovedForListing function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCurrencyApprovedForListing } from "thirdweb/extensions/marketplace";
 *
 * const result = await isCurrencyApprovedForListing({
 *  contract,
 *  listingId: ...,
 *  currency: ...,
 * });
 *
 * ```
 */
export declare function isCurrencyApprovedForListing(options: BaseTransactionOptions<IsCurrencyApprovedForListingParams>): Promise<boolean>;
//# sourceMappingURL=isCurrencyApprovedForListing.d.ts.map