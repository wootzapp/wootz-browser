import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "depositTo" function.
 */
export type DepositToParams = WithOverrides<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
}>;
export declare const FN_SELECTOR: "0xb760faf9";
/**
 * Checks if the `depositTo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `depositTo` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isDepositToSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isDepositToSupported(["0x..."]);
 * ```
 */
export declare function isDepositToSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "depositTo" function.
 * @param options - The options for the depositTo function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeDepositToParams } from "thirdweb/extensions/erc4337";
 * const result = encodeDepositToParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeDepositToParams(options: DepositToParams): `0x${string}`;
/**
 * Encodes the "depositTo" function into a Hex string with its parameters.
 * @param options - The options for the depositTo function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeDepositTo } from "thirdweb/extensions/erc4337";
 * const result = encodeDepositTo({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeDepositTo(options: DepositToParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "depositTo" function on the contract.
 * @param options - The options for the "depositTo" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { depositTo } from "thirdweb/extensions/erc4337";
 *
 * const transaction = depositTo({
 *  contract,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function depositTo(options: BaseTransactionOptions<DepositToParams | {
    asyncParams: () => Promise<DepositToParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=depositTo.d.ts.map