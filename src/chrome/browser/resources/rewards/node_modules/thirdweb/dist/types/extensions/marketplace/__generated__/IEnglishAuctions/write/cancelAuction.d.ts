import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "cancelAuction" function.
 */
export type CancelAuctionParams = WithOverrides<{
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
}>;
export declare const FN_SELECTOR: "0x96b5a755";
/**
 * Checks if the `cancelAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `cancelAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCancelAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCancelAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCancelAuctionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "cancelAuction" function.
 * @param options - The options for the cancelAuction function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelAuctionParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelAuctionParams({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCancelAuctionParams(options: CancelAuctionParams): `0x${string}`;
/**
 * Encodes the "cancelAuction" function into a Hex string with its parameters.
 * @param options - The options for the cancelAuction function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCancelAuction } from "thirdweb/extensions/marketplace";
 * const result = encodeCancelAuction({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeCancelAuction(options: CancelAuctionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "cancelAuction" function on the contract.
 * @param options - The options for the "cancelAuction" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { cancelAuction } from "thirdweb/extensions/marketplace";
 *
 * const transaction = cancelAuction({
 *  contract,
 *  auctionId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function cancelAuction(options: BaseTransactionOptions<CancelAuctionParams | {
    asyncParams: () => Promise<CancelAuctionParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=cancelAuction.d.ts.map