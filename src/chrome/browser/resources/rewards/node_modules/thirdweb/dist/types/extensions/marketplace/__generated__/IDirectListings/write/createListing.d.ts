import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createListing" function.
 */
export type CreateListingParams = WithOverrides<{
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
                name: "pricePerToken";
            },
            {
                type: "uint128";
                name: "startTimestamp";
            },
            {
                type: "uint128";
                name: "endTimestamp";
            },
            {
                type: "bool";
                name: "reserved";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x746415b5";
/**
 * Checks if the `createListing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createListing` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isCreateListingSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isCreateListingSupported(["0x..."]);
 * ```
 */
export declare function isCreateListingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createListing" function.
 * @param options - The options for the createListing function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCreateListingParams } from "thirdweb/extensions/marketplace";
 * const result = encodeCreateListingParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCreateListingParams(options: CreateListingParams): `0x${string}`;
/**
 * Encodes the "createListing" function into a Hex string with its parameters.
 * @param options - The options for the createListing function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeCreateListing } from "thirdweb/extensions/marketplace";
 * const result = encodeCreateListing({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeCreateListing(options: CreateListingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createListing" function on the contract.
 * @param options - The options for the "createListing" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createListing } from "thirdweb/extensions/marketplace";
 *
 * const transaction = createListing({
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
export declare function createListing(options: BaseTransactionOptions<CreateListingParams | {
    asyncParams: () => Promise<CreateListingParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createListing.d.ts.map