import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "offer" function.
 */
export type OfferParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    quantityWanted: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_quantityWanted";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currency";
    }>;
    pricePerToken: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_pricePerToken";
    }>;
    expirationTimestamp: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_expirationTimestamp";
    }>;
}>;
export declare const FN_SELECTOR: "0x5fef45e7";
/**
 * Checks if the `offer` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `offer` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isOfferSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isOfferSupported(["0x..."]);
 * ```
 */
export declare function isOfferSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "offer" function.
 * @param options - The options for the offer function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeOfferParams } from "thirdweb/extensions/marketplace";
 * const result = encodeOfferParams({
 *  listingId: ...,
 *  quantityWanted: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  expirationTimestamp: ...,
 * });
 * ```
 */
export declare function encodeOfferParams(options: OfferParams): `0x${string}`;
/**
 * Encodes the "offer" function into a Hex string with its parameters.
 * @param options - The options for the offer function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeOffer } from "thirdweb/extensions/marketplace";
 * const result = encodeOffer({
 *  listingId: ...,
 *  quantityWanted: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  expirationTimestamp: ...,
 * });
 * ```
 */
export declare function encodeOffer(options: OfferParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "offer" function on the contract.
 * @param options - The options for the "offer" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { offer } from "thirdweb/extensions/marketplace";
 *
 * const transaction = offer({
 *  contract,
 *  listingId: ...,
 *  quantityWanted: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  expirationTimestamp: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function offer(options: BaseTransactionOptions<OfferParams | {
    asyncParams: () => Promise<OfferParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=offer.d.ts.map