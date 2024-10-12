import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "getRoyalty" function.
 */
export type GetRoyaltyParams = WithOverrides<{
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
}>;
export declare const FN_SELECTOR: "0xf533b802";
/**
 * Checks if the `getRoyalty` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getRoyalty` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isGetRoyaltySupported } from "thirdweb/extensions/common";
 *
 * const supported = isGetRoyaltySupported(["0x..."]);
 * ```
 */
export declare function isGetRoyaltySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getRoyalty" function.
 * @param options - The options for the getRoyalty function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeGetRoyaltyParams } from "thirdweb/extensions/common";
 * const result = encodeGetRoyaltyParams({
 *  tokenAddress: ...,
 *  tokenId: ...,
 *  value: ...,
 * });
 * ```
 */
export declare function encodeGetRoyaltyParams(options: GetRoyaltyParams): `0x${string}`;
/**
 * Encodes the "getRoyalty" function into a Hex string with its parameters.
 * @param options - The options for the getRoyalty function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeGetRoyalty } from "thirdweb/extensions/common";
 * const result = encodeGetRoyalty({
 *  tokenAddress: ...,
 *  tokenId: ...,
 *  value: ...,
 * });
 * ```
 */
export declare function encodeGetRoyalty(options: GetRoyaltyParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "getRoyalty" function on the contract.
 * @param options - The options for the "getRoyalty" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { getRoyalty } from "thirdweb/extensions/common";
 *
 * const transaction = getRoyalty({
 *  contract,
 *  tokenAddress: ...,
 *  tokenId: ...,
 *  value: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function getRoyalty(options: BaseTransactionOptions<GetRoyaltyParams | {
    asyncParams: () => Promise<GetRoyaltyParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=getRoyalty.d.ts.map