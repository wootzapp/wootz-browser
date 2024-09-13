import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "withdrawTo" function.
 */
export type WithdrawToParams = WithOverrides<{
    withdrawAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "withdrawAddress";
    }>;
    withdrawAmount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "withdrawAmount";
    }>;
}>;
export declare const FN_SELECTOR: "0x205c2878";
/**
 * Checks if the `withdrawTo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `withdrawTo` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isWithdrawToSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isWithdrawToSupported(["0x..."]);
 * ```
 */
export declare function isWithdrawToSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "withdrawTo" function.
 * @param options - The options for the withdrawTo function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeWithdrawToParams } from "thirdweb/extensions/erc4337";
 * const result = encodeWithdrawToParams({
 *  withdrawAddress: ...,
 *  withdrawAmount: ...,
 * });
 * ```
 */
export declare function encodeWithdrawToParams(options: WithdrawToParams): `0x${string}`;
/**
 * Encodes the "withdrawTo" function into a Hex string with its parameters.
 * @param options - The options for the withdrawTo function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeWithdrawTo } from "thirdweb/extensions/erc4337";
 * const result = encodeWithdrawTo({
 *  withdrawAddress: ...,
 *  withdrawAmount: ...,
 * });
 * ```
 */
export declare function encodeWithdrawTo(options: WithdrawToParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "withdrawTo" function on the contract.
 * @param options - The options for the "withdrawTo" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { withdrawTo } from "thirdweb/extensions/erc4337";
 *
 * const transaction = withdrawTo({
 *  contract,
 *  withdrawAddress: ...,
 *  withdrawAmount: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function withdrawTo(options: BaseTransactionOptions<WithdrawToParams | {
    asyncParams: () => Promise<WithdrawToParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=withdrawTo.d.ts.map