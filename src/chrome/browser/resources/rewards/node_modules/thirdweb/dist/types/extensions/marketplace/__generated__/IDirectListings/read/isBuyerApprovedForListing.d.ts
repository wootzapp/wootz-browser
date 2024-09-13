import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isBuyerApprovedForListing" function.
 */
export type IsBuyerApprovedForListingParams = {
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    buyer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_buyer";
    }>;
};
export declare const FN_SELECTOR: "0x9cfbe2a6";
/**
 * Checks if the `isBuyerApprovedForListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isBuyerApprovedForListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isIsBuyerApprovedForListingSupported } from "thirdweb/extensions/marketplace";
 * const supported = isIsBuyerApprovedForListingSupported(["0x..."]);
 * ```
 */
export declare function isIsBuyerApprovedForListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isBuyerApprovedForListing" function.
 * @param options - The options for the isBuyerApprovedForListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsBuyerApprovedForListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeIsBuyerApprovedForListingParams({
 *  listingId: ...,
 *  buyer: ...,
 * });
 * ```
 */
export declare function encodeIsBuyerApprovedForListingParams(options: IsBuyerApprovedForListingParams): `0x${string}`;
/**
 * Encodes the "isBuyerApprovedForListing" function into a Hex string with its parameters.
 * @param options - The options for the isBuyerApprovedForListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsBuyerApprovedForListing } from "thirdweb/extensions/marketplace";
 * const result = encodeIsBuyerApprovedForListing({
 *  listingId: ...,
 *  buyer: ...,
 * });
 * ```
 */
export declare function encodeIsBuyerApprovedForListing(options: IsBuyerApprovedForListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isBuyerApprovedForListing function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeIsBuyerApprovedForListingResult } from "thirdweb/extensions/marketplace";
 * const result = decodeIsBuyerApprovedForListingResultResult("...");
 * ```
 */
export declare function decodeIsBuyerApprovedForListingResult(result: Hex): boolean;
/**
 * Calls the "isBuyerApprovedForListing" function on the contract.
 * @param options - The options for the isBuyerApprovedForListing function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isBuyerApprovedForListing } from "thirdweb/extensions/marketplace";
 *
 * const result = await isBuyerApprovedForListing({
 *  contract,
 *  listingId: ...,
 *  buyer: ...,
 * });
 *
 * ```
 */
export declare function isBuyerApprovedForListing(options: BaseTransactionOptions<IsBuyerApprovedForListingParams>): Promise<boolean>;
//# sourceMappingURL=isBuyerApprovedForListing.d.ts.map