import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "cancelOffer" function.
 */
export type CancelOfferParams = WithOverrides<{
    offerId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_offerId";
    }>;
}>;
export declare const FN_SELECTOR: "0xef706adf";
/**
 * Checks if the `cancelOffer` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `cancelOffer` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCancelOfferSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCancelOfferSupported(["0x..."]);
 * ```
 */
export declare function isCancelOfferSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "cancelOffer" function.
 * @param options - The options for the cancelOffer function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelOfferParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelOfferParams({
 *  offerId: ...,
 * });
 * ```
 */
export declare function encodeCancelOfferParams(options: CancelOfferParams): `0x${string}`;
/**
 * Encodes the "cancelOffer" function into a Hex string with its parameters.
 * @param options - The options for the cancelOffer function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelOffer } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelOffer({
 *  offerId: ...,
 * });
 * ```
 */
export declare function encodeCancelOffer(options: CancelOfferParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "cancelOffer" function on the contract.
 * @param options - The options for the "cancelOffer" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { cancelOffer } from "thirdweb/extensions/marketplace";
 *
 * const transaction = cancelOffer({
 *  contract,
 *  offerId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function cancelOffer(options: BaseTransactionOptions<CancelOfferParams | {
    asyncParams: () => Promise<CancelOfferParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=cancelOffer.d.ts.map