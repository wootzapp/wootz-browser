import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "acceptOffer" function.
 */
export type AcceptOfferParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    offeror: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_offeror";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_currency";
    }>;
    totalPrice: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_totalPrice";
    }>;
}>;
export declare const FN_SELECTOR: "0xb13c0e63";
/**
 * Checks if the `acceptOffer` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `acceptOffer` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isAcceptOfferSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isAcceptOfferSupported(["0x..."]);
 * ```
 */
export declare function isAcceptOfferSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "acceptOffer" function.
 * @param options - The options for the acceptOffer function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeAcceptOfferParams } from "thirdweb/extensions/marketplace";
 * const result = encodeAcceptOfferParams({
 *  listingId: ...,
 *  offeror: ...,
 *  currency: ...,
 *  totalPrice: ...,
 * });
 * ```
 */
export declare function encodeAcceptOfferParams(options: AcceptOfferParams): `0x${string}`;
/**
 * Encodes the "acceptOffer" function into a Hex string with its parameters.
 * @param options - The options for the acceptOffer function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeAcceptOffer } from "thirdweb/extensions/marketplace";
 * const result = encodeAcceptOffer({
 *  listingId: ...,
 *  offeror: ...,
 *  currency: ...,
 *  totalPrice: ...,
 * });
 * ```
 */
export declare function encodeAcceptOffer(options: AcceptOfferParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "acceptOffer" function on the contract.
 * @param options - The options for the "acceptOffer" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { acceptOffer } from "thirdweb/extensions/marketplace";
 *
 * const transaction = acceptOffer({
 *  contract,
 *  listingId: ...,
 *  offeror: ...,
 *  currency: ...,
 *  totalPrice: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function acceptOffer(options: BaseTransactionOptions<AcceptOfferParams | {
    asyncParams: () => Promise<AcceptOfferParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=acceptOffer.d.ts.map