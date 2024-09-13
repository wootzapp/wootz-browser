import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "cancelListing" function.
 */
export type CancelListingParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
}>;
export declare const FN_SELECTOR: "0x305a67a8";
/**
 * Checks if the `cancelListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `cancelListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCancelListingSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCancelListingSupported(["0x..."]);
 * ```
 */
export declare function isCancelListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "cancelListing" function.
 * @param options - The options for the cancelListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelListingParams({
 *  listingId: ...,
 * });
 * ```
 */
export declare function encodeCancelListingParams(options: CancelListingParams): `0x${string}`;
/**
 * Encodes the "cancelListing" function into a Hex string with its parameters.
 * @param options - The options for the cancelListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelListing } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelListing({
 *  listingId: ...,
 * });
 * ```
 */
export declare function encodeCancelListing(options: CancelListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "cancelListing" function on the contract.
 * @param options - The options for the "cancelListing" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { cancelListing } from "thirdweb/extensions/marketplace";
 *
 * const transaction = cancelListing({
 *  contract,
 *  listingId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function cancelListing(options: BaseTransactionOptions<CancelListingParams | {
    asyncParams: () => Promise<CancelListingParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=cancelListing.d.ts.map