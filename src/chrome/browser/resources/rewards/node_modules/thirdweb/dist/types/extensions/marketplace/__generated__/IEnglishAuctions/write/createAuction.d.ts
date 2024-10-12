import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createAuction" function.
 */
export type CreateAuctionParams = WithOverrides<{
    params: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "_params";
        components: [
            {
                type: "address";
                name: "assetContract";
            },
            {
                type: "uint256";
                name: "tokenId";
            },
            {
                type: "uint256";
                name: "quantity";
            },
            {
                type: "address";
                name: "currency";
            },
            {
                type: "uint256";
                name: "minimumBidAmount";
            },
            {
                type: "uint256";
                name: "buyoutBidAmount";
            },
            {
                type: "uint64";
                name: "timeBufferInSeconds";
            },
            {
                type: "uint64";
                name: "bidBufferBps";
            },
            {
                type: "uint64";
                name: "startTimestamp";
            },
            {
                type: "uint64";
                name: "endTimestamp";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x16654d40";
/**
 * Checks if the `createAuction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createAuction` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCreateAuctionSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCreateAuctionSupported(["0x..."]);
 * ```
 */
export declare function isCreateAuctionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createAuction" function.
 * @param options - The options for the createAuction function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCreateAuctionParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCreateAuctionParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCreateAuctionParams(options: CreateAuctionParams): `0x${string}`;
/**
 * Encodes the "createAuction" function into a Hex string with its parameters.
 * @param options - The options for the createAuction function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCreateAuction } from "thirdweb/extensions/marketplace";
 * const result = encodeCreateAuction({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCreateAuction(options: CreateAuctionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createAuction" function on the contract.
 * @param options - The options for the "createAuction" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createAuction } from "thirdweb/extensions/marketplace";
 *
 * const transaction = createAuction({
 *  contract,
 *  params: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createAuction(options: BaseTransactionOptions<CreateAuctionParams | {
    asyncParams: () => Promise<CreateAuctionParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createAuction.d.ts.map