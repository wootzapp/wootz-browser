import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "closeAuction" function.
 */
export type CloseAuctionParams = WithOverrides<{
    listingId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_listingId";
    }>;
    closeFor: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_closeFor";
    }>;
}>;
export declare const FN_SELECTOR: "0x6bab66ae";
/**
 * Checks if the `closeAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `closeAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCloseAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCloseAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCloseAuctionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "closeAuction" function.
 * @param options - The options for the closeAuction function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCloseAuctionParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCloseAuctionParams({
 *  listingId: ...,
 *  closeFor: ...,
 * });
 * ```
 */
export declare function encodeCloseAuctionParams(options: CloseAuctionParams): `0x${string}`;
/**
 * Encodes the "closeAuction" function into a Hex string with its parameters.
 * @param options - The options for the closeAuction function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCloseAuction } from "thirdweb/extensions/marketplace";
 * const result = encodeCloseAuction({
 *  listingId: ...,
 *  closeFor: ...,
 * });
 * ```
 */
export declare function encodeCloseAuction(options: CloseAuctionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "closeAuction" function on the contract.
 * @param options - The options for the "closeAuction" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { closeAuction } from "thirdweb/extensions/marketplace";
 *
 * const transaction = closeAuction({
 *  contract,
 *  listingId: ...,
 *  closeFor: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function closeAuction(options: BaseTransactionOptions<CloseAuctionParams | {
    asyncParams: () => Promise<CloseAuctionParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=closeAuction.d.ts.map