import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "updateListing" function.
 */
export type UpdateListingParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    quantityToList: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_quantityToList";
    }>;
    reservePricePerToken: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_reservePricePerToken";
    }>;
    buyoutPricePerToken: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_buyoutPricePerToken";
    }>;
    currencyToAccept: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currencyToAccept";
    }>;
    startTime: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_startTime";
    }>;
    secondsUntilEndTime: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_secondsUntilEndTime";
    }>;
}>;
export declare const FN_SELECTOR: "0xc4b5b15f";
/**
 * Checks if the `updateListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `updateListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isUpdateListingSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isUpdateListingSupported(["0x..."]);
 * ```
 */
export declare function isUpdateListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "updateListing" function.
 * @param options - The options for the updateListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeUpdateListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeUpdateListingParams({
 *  listingId: ...,
 *  quantityToList: ...,
 *  reservePricePerToken: ...,
 *  buyoutPricePerToken: ...,
 *  currencyToAccept: ...,
 *  startTime: ...,
 *  secondsUntilEndTime: ...,
 * });
 * ```
 */
export declare function encodeUpdateListingParams(options: UpdateListingParams): `0x${string}`;
/**
 * Encodes the "updateListing" function into a Hex string with its parameters.
 * @param options - The options for the updateListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeUpdateListing } from "thirdweb/extensions/marketplace";
 * const result = encodeUpdateListing({
 *  listingId: ...,
 *  quantityToList: ...,
 *  reservePricePerToken: ...,
 *  buyoutPricePerToken: ...,
 *  currencyToAccept: ...,
 *  startTime: ...,
 *  secondsUntilEndTime: ...,
 * });
 * ```
 */
export declare function encodeUpdateListing(options: UpdateListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "updateListing" function on the contract.
 * @param options - The options for the "updateListing" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { updateListing } from "thirdweb/extensions/marketplace";
 *
 * const transaction = updateListing({
 *  contract,
 *  listingId: ...,
 *  quantityToList: ...,
 *  reservePricePerToken: ...,
 *  buyoutPricePerToken: ...,
 *  currencyToAccept: ...,
 *  startTime: ...,
 *  secondsUntilEndTime: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function updateListing(options: BaseTransactionOptions<UpdateListingParams | {
    asyncParams: () => Promise<UpdateListingParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=updateListing.d.ts.map