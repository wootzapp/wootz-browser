import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "makeOffer" function.
 */
export type MakeOfferParams = WithOverrides<{
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
                name: "totalPrice";
            },
            {
                type: "uint256";
                name: "expirationTimestamp";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x016767fa";
/**
 * Checks if the `makeOffer` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `makeOffer` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isMakeOfferSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isMakeOfferSupported(["0x..."]);
 * ```
 */
export declare function isMakeOfferSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "makeOffer" function.
 * @param options - The options for the makeOffer function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeMakeOfferParams } from "thirdweb/extensions/marketplace";
 * const result = encodeMakeOfferParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeMakeOfferParams(options: MakeOfferParams): `0x${string}`;
/**
 * Encodes the "makeOffer" function into a Hex string with its parameters.
 * @param options - The options for the makeOffer function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeMakeOffer } from "thirdweb/extensions/marketplace";
 * const result = encodeMakeOffer({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeMakeOffer(options: MakeOfferParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "makeOffer" function on the contract.
 * @param options - The options for the "makeOffer" function.
 * @returns A prepared transaction object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { makeOffer } from "thirdweb/extensions/marketplace";
 *
 * const transaction = makeOffer({
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
export declare function makeOffer(options: BaseTransactionOptions<MakeOfferParams | {
    asyncParams: () => Promise<MakeOfferParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=makeOffer.d.ts.map