import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "approveCurrencyForListing" function.
 */
export type ApproveCurrencyForListingParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currency";
    }>;
    pricePerTokenInCurrency: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_pricePerTokenInCurrency";
    }>;
}>;
export declare const FN_SELECTOR: "0xea8f9a3c";
/**
 * Checks if the `approveCurrencyForListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `approveCurrencyForListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isApproveCurrencyForListingSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isApproveCurrencyForListingSupported(["0x..."]);
 * ```
 */
export declare function isApproveCurrencyForListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "approveCurrencyForListing" function.
 * @param options - The options for the approveCurrencyForListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeApproveCurrencyForListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeApproveCurrencyForListingParams({
 *  listingId: ...,
 *  currency: ...,
 *  pricePerTokenInCurrency: ...,
 * });
 * ```
 */
export declare function encodeApproveCurrencyForListingParams(options: ApproveCurrencyForListingParams): `0x${string}`;
/**
 * Encodes the "approveCurrencyForListing" function into a Hex string with its parameters.
 * @param options - The options for the approveCurrencyForListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeApproveCurrencyForListing } from "thirdweb/extensions/marketplace";
 * const result = encodeApproveCurrencyForListing({
 *  listingId: ...,
 *  currency: ...,
 *  pricePerTokenInCurrency: ...,
 * });
 * ```
 */
export declare function encodeApproveCurrencyForListing(options: ApproveCurrencyForListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "approveCurrencyForListing" function on the contract.
 * @param options - The options for the "approveCurrencyForListing" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { approveCurrencyForListing } from "thirdweb/extensions/marketplace";
 *
 * const transaction = approveCurrencyForListing({
 *  contract,
 *  listingId: ...,
 *  currency: ...,
 *  pricePerTokenInCurrency: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function approveCurrencyForListing(options: BaseTransactionOptions<ApproveCurrencyForListingParams | {
    asyncParams: () => Promise<ApproveCurrencyForListingParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=approveCurrencyForListing.d.ts.map